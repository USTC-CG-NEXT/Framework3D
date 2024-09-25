import os

def replace_text_in_file(file_path, old_text, new_text):
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()
    
    content = content.replace(old_text, new_text)
    
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(content)

def recursively_replace_text_in_folder(folder_path, old_text, new_text):
    for root, _, files in os.walk(folder_path):
        for file in files:
            file_path = os.path.join(root, file)
            replace_text_in_file(file_path, old_text, new_text)

if __name__ == "__main__":
    current_folder = os.path.dirname(os.path.abspath(__file__))
    recursively_replace_text_in_folder(current_folder, 'nvrhi::DeviceHandle', 'nvrhi::DeviceHandle')