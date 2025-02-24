void mx_mix_surfaceshader(surfaceshader fg, surfaceshader bg, float w, out surfaceshader returnshader)
{
    returnshader.color = lerp(bg.color, fg.color, w);
    returnshader.transparency = lerp(bg.transparency, fg.transparency, w);
}
