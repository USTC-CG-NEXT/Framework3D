import torch
import math


def AbsCosTheta(w):
    return torch.abs(w[:, 2])


def Lum(color):
    YWeight = torch.tensor(
        [0.212671, 0.715160, 0.072169], dtype=color.dtype, device=color.device
    )
    return torch.sum(color * YWeight, dim=1)


def SchlickR0FromEta(eta):
    return (eta - 1) ** 2 / (eta + 1) ** 2


def Cos2Theta(w):
    return w[:, 2] ** 2


def Sin2Theta(w):
    return 1 - Cos2Theta(w)


def Tan2Theta(w):
    return Sin2Theta(w) / Cos2Theta(w)


def SinTheta(w):
    return torch.sqrt(Sin2Theta(w))


def CosTheta(w):
    return w[:, 2]


def TanTheta(w):
    return SinTheta(w) / CosTheta(w)


def CosPhi(w):
    sinTheta = SinTheta(w)
    tmp = torch.clamp(w[:, 0] / sinTheta, -1, 1)
    result = torch.where(
        sinTheta == 0, torch.tensor(0.0, dtype=w.dtype, device=w.device), tmp
    )
    return result


def SinPhi(w):
    sinTheta = SinTheta(w)
    tmp = torch.clamp(w[:, 1] / sinTheta, -1, 1)
    result = torch.where(
        sinTheta == 0, torch.tensor(0.0, dtype=w.dtype, device=w.device), tmp
    )
    return result


def Cos2Phi(w):
    return CosPhi(w) ** 2


def Sin2Phi(w):
    return SinPhi(w) ** 2


def SchlickWeight(cosTheta):
    m = torch.clamp(1 - cosTheta, 0, 1)
    return m**5


def lerp(v0, v1, t):
    t = t.unsqueeze(-1) if t.dim() == 1 else t
    return (1 - t) * v0 + t * v1


def FrSchlick(R0, cosTheta):
    return lerp(
        R0,
        torch.tensor([1.0, 1.0, 1.0], dtype=R0.dtype, device=R0.device),
        SchlickWeight(cosTheta),
    )


def DisneyFresnel(R0, metallic, eta, cosI):
    return FrSchlick(R0, cosI)


def Faceforward(v1, v2):
    tmp = torch.sum(v1 * v2, dim=1, keepdim=True)
    result = torch.where(tmp < 0, -v1, v1)
    return result


def Microfacet_G1(w, param):
    absTanTheta = torch.abs(TanTheta(w))
    alpha = torch.sqrt(Cos2Phi(w) * param[:, 0] ** 2 + Sin2Phi(w) * param[:, 1] ** 2)
    alpha2Tan2Theta = (alpha * absTanTheta) ** 2
    lambda_ = (-1 + torch.sqrt(1 + alpha2Tan2Theta)) / 2
    return 1 / (1 + lambda_)


def Microfacet_G(wi, wo, param):
    return Microfacet_G1(wi, param) * Microfacet_G1(wo, param)


def MakeMicroPara(roughness):
    ax = torch.clamp(torch.sqrt(roughness), min=0.001)
    ay = torch.clamp(torch.sqrt(roughness), min=0.001)
    return torch.stack([ax, ay], dim=1)


def MicrofacetDistribution(wh, param):
    tan2Theta = Tan2Theta(wh)
    cos4Theta = Cos2Theta(wh) ** 2
    e = (
        Cos2Phi(wh) / (param[:, 0] ** 2) + Sin2Phi(wh) / (param[:, 1] ** 2)
    ) * tan2Theta
    return 1 / (math.pi * param[:, 0] * param[:, 1] * cos4Theta * (1 + e) ** 2)


def bsdf_f(ray_in_d, ray_out_d, roughness, baseColor=None):
    if baseColor is None:
        baseColor = (
            torch.tensor([1.0, 1.0, 1.0], dtype=ray_in_d.dtype, device=ray_in_d.device)
            .unsqueeze(0)
            .repeat(ray_in_d.shape[0], 1)
        )

    micro_para = MakeMicroPara(roughness)
    wo = torch.nn.functional.normalize(ray_in_d, dim=1)
    wi = torch.nn.functional.normalize(ray_out_d, dim=1)

    wo = torch.where(wo[:, 2].unsqueeze(1) < 0, -wo, wo)
    wi = torch.where(wi[:, 2].unsqueeze(1) < 0, -wi, wi)

    cosThetaO = AbsCosTheta(wo)
    cosThetaI = AbsCosTheta(wi)
    wh = wi + wo

    if torch.any(cosThetaI == 0) or torch.any(cosThetaO == 0):
        return torch.zeros_like(cosThetaI)

    if torch.all(wh == 0):
        return torch.zeros_like(cosThetaI)

    wh = torch.nn.functional.normalize(wh, dim=1)
    lum = Lum(baseColor)

    Ctint = torch.where(
        lum.unsqueeze(1) > 0,
        baseColor / lum.unsqueeze(1),
        torch.tensor([1.0, 1.0, 1.0], dtype=baseColor.dtype, device=baseColor.device),
    )

    Cspec0 = baseColor
    F = DisneyFresnel(
        Cspec0,
        1.0,
        0.0,
        torch.sum(
            wi
            * Faceforward(
                wh,
                torch.tensor([0.0, 0.0, 1.0], dtype=wh.dtype, device=wh.device)
                .unsqueeze(0)
                .repeat(wh.shape[0], 1),
            ),
            dim=1,
        ),
    )

    return (
        lum
        * MicrofacetDistribution(wh, micro_para)
        * Microfacet_G(wo, wi, micro_para)
        * Lum(F)
        / (4 * cosThetaI * cosThetaO)
    )


def bsdf_f_line(ray_in_d, ray_out_d, roughness, baseColor=None):
    if baseColor is None:
        baseColor = (
            torch.tensor([1.0, 1.0, 1.0], dtype=ray_in_d.dtype, device=ray_in_d.device)
            .unsqueeze(0)
            .repeat(ray_in_d.shape[0], 1)
        )

    micro_para = MakeMicroPara(roughness)
    wo = torch.nn.functional.normalize(ray_in_d, dim=1)
    wi = torch.nn.functional.normalize(ray_out_d, dim=1)

    wo = torch.where(wo[:, 2].unsqueeze(1) < 0, -wo, wo)
    wi = torch.where(wi[:, 2].unsqueeze(1) < 0, -wi, wi)

    cosThetaO = AbsCosTheta(wo)
    cosThetaI = AbsCosTheta(wi)
    wh = wi + wo

    if torch.any(cosThetaI == 0) or torch.any(cosThetaO == 0):
        return torch.zeros_like(cosThetaI)

    if torch.all(wh == 0):
        return torch.zeros_like(cosThetaI)

    wh = torch.nn.functional.normalize(wh, dim=1)
    lum = Lum(baseColor)

    Ctint = torch.where(
        lum.unsqueeze(1) > 0,
        baseColor / lum.unsqueeze(1),
        torch.tensor([1.0, 1.0, 1.0], dtype=baseColor.dtype, device=baseColor.device),
    )

    Cspec0 = baseColor
    F = DisneyFresnel(
        Cspec0,
        1.0,
        0.0,
        torch.sum(
            wi
            * Faceforward(
                wh,
                torch.tensor([0.0, 0.0, 1.0], dtype=wh.dtype, device=wh.device)
                .unsqueeze(0)
                .repeat(wh.shape[0], 1),
            ),
            dim=1,
        ),
    )

    return lum * Microfacet_G(wo, wi, micro_para) * Lum(F) / (4 * cosThetaI * cosThetaO)
