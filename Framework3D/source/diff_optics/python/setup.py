import slangtorch
import os


shaders_path = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(__file__)))),
    "Binaries",
    "Debug",
)

slang_include_path = "C:/Users/Jerry/scoop/apps/slang/2024.1.22"


import torch

shader_name = shaders_path + "/square.slang"

m = slangtorch.loadModule(shader_name, includePaths=[slang_include_path])

input = torch.tensor((0, 1, 2, 3, 4, 5), dtype=torch.float).cuda()
output = torch.zeros_like(input).cuda()

# Invoke normally
m.square(input=input, output=output).launchRaw(blockSize=(6, 1, 1), gridSize=(1, 1, 1))

print(output)

# Invoke reverse-mode autodiff by first allocating tensors to hold the gradients
input = torch.tensor((0, 1, 2, 3, 4, 5), dtype=torch.float).cuda()
input_grad = torch.zeros_like(input).cuda()

output = torch.zeros_like(input)
# Pass in all 1s as the output derivative for our example
output_grad = torch.ones_like(output)

m.square.bwd(input=(input, input_grad), output=(output, output_grad)).launchRaw(
    blockSize=(6, 1, 1), gridSize=(1, 1, 1)
)

# Derivatives get propagated to input_grad
print(input_grad)

# Note that the derivatives in output_grad are 'consumed'.
# i.e. all zeros after the call.
print(output_grad)
