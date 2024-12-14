import torch
import torch.nn as nn
import torch.nn.functional as F


class Sandwich(nn.Module):
    def __init__(self, bias=False):
        super(Sandwich, self).__init__()

        self.mlp1 = nn.Conv2d(12, 6, kernel_size=1, bias=bias)  #
        self.mlp2 = nn.Conv2d(6, 3, kernel_size=1, bias=bias)

    def forward(self, input, rays):
        albedo, spec, timefeature = input.chunk(3, dim=1)
        specular = torch.cat([spec, timefeature, rays], dim=1)
        specular = self.mlp2(F.relu(self.mlp1(specular)))

        result = albedo + specular
        result = torch.sigmoid(result)
        return result


class SandwichEvent(nn.Module):
    def __init__(self, bias=True):
        super(SandwichEvent, self).__init__()
        pass  # not used for now

    def forward(self, x):
        return x


def declare_node():
    return [
        {
            "pt_model_path": "String",
            "features": "TorchTensor",
            "rays": "TorchTensor",
        },
        {"output": "TorchTensor"},
    ]


def exec_node(pt_model_path: str, features: torch.Tensor, rays: torch.Tensor):

    model = Sandwich()
    loaded = torch.load(pt_model_path)
    loaded["model"]["mlp1.weight"] = (
        loaded["model"]["decoder.mlp1.weight"].detach().cpu()
    )
    loaded["model"]["mlp2.weight"] = (
        loaded["model"]["decoder.mlp2.weight"].detach().cpu()
    )

    model_dict = model.state_dict()
    state_dict = {
        k: v for k, v in loaded["model"].items() if k in model.state_dict().keys()
    }

    model_dict.update(state_dict)
    model.load_state_dict(model_dict)

    model.eval()

    evaluated = model(features, rays)
    # evaluated = features[:, :3, :, :] 

    rgb = torch.abs(evaluated[0, :, :, :].permute(1, 2, 0))
    # add one channel of 1 to the rendered image
    rendered_image = torch.cat(
        (rgb , torch.ones_like(rgb[:, :, :1])), dim=2
    )

    return rendered_image


def wrap_exec(list):
    return exec_node(*list)
