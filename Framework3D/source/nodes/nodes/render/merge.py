import os

replacements = 0

def replace_text_in_file(file_path, replacements):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        
        new_content = content
        for old_text, new_text in replacements:
            new_content = new_content.replace(old_text, new_text)
        
        if new_content != content:
            with open(file_path, 'w', encoding='utf-8') as file:
                file.write(new_content)
    except UnicodeDecodeError:
        print(f"Skipping file {file_path} due to encoding error.")

def recursively_replace_text_in_folder(folder_path, replacements, script_name):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file == script_name:
                continue
            file_path = os.path.join(root, file)
            replace_text_in_file(file_path, replacements)

if __name__ == "__main__":
    current_folder = os.path.dirname(os.path.abspath(__file__))
    script_name = os.path.basename(__file__)
    replacements = [
        ('ref<Device>', 'nvrhi::DeviceHandle'),
        ('ref<Buffer>', 'nvrhi::BufferHandle'),
        ('ref<Texture>', 'nvrhi::TextureHandle'),
        ('ref<Sampler>', 'nvrhi::SamplerHandle'),
        ('ref<RtAccelerationStructure>', 'nvrhi::rt::AccelerationStructure'),
        ('"fstd/bit.h"', '<bit>'),
    ]
    recursively_replace_text_in_folder(current_folder, replacements, script_name)