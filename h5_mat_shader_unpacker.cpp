// h5_mat_shader_unpacker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "Tag stuff/TagFramework.h"
#include "Tag stuff/ShaderByteCodeResource.h"


#include <filesystem>
using namespace std::experimental::filesystem;
std::vector<std::string> get_shaderbytecoderesources(const std::string& dirPath) {
    std::vector<std::string> listOfFiles;
    try {
        if (filesystem::exists(dirPath) && filesystem::is_directory(dirPath)) {
            filesystem::recursive_directory_iterator iter(dirPath);
            filesystem::recursive_directory_iterator end;
            while (iter != end) {
                // Check if current entry is a directory and if exists in skip list
                if (!filesystem::is_directory(iter->path()) && iter->path().string().ends_with("_shader bytecode resources]"))
                    listOfFiles.push_back(iter->path().string());
                error_code ec;
                iter.increment(ec);
                if (ec) std::cerr << "Error While Accessing : " << iter->path().string() << " :: " << ec.message() << '\n';
            }
        }
    }
    catch (std::system_error& e) { std::cerr << "Exception :: " << e.what(); }

    return listOfFiles;
}




static string dir;
static string output;

void handle_compiled_shader_block(__base_compiled_shader* shader_block, string filename, string type, int index) {
    // output data
    ofstream data_file(output + "\\" + filename + "_" + type + "[" + to_string(index) + "]" + ".bin");
    data_file.write(shader_block->dx11_compiled_shader____dx11_compiled_shader_.content_ptr, shader_block->dx11_compiled_shader____dx11_compiled_shader_.data_size);
}
string ExtractFilename(string file){
    size_t slash_pos = file.find_last_of('\\');
    if (slash_pos != string::npos)
        file = file.substr(slash_pos);

    size_t ext_pos = file.find_last_of('.');
    if (ext_pos != string::npos)
        file = file.substr(0, ext_pos);

    return file;
}

int main()
{
    std::cout << "Hello World!\n";
    std::cout << "Enter directory of material shader bank files\n";

    //std::cin >> dir;
    dir = "C:\\Users\\Joe bingle\\Downloads\\Shaderbanks\\Shaderbanks";

    //std::cin >> output;
    output = "C:\\Users\\Joe bingle\\Downloads\\Shaderbanks\\Output";



    auto files = get_shaderbytecoderesources(dir);
    for (int i = 0; i < files.size(); i++) {

        std::ifstream file_stream(files[i], std::ios::binary | std::ios::ate);
        if (!file_stream.is_open())
            continue; //throw exception("bad file");

        std::streamsize file_size = file_stream.tellg();
        if (file_size < tag_header_size) {
            file_stream.close();
            continue; //throw exception("bad file size");
        }

        // read the whole file
        char* tag_bytes = new char[file_size]; // this gets cleaned up in open_ready_tag;
        file_stream.seekg(0, std::ios::beg);
        file_stream.read(tag_bytes, file_size);
        file_stream.close();

        // process file into runtime tag
        char* tagdata = nullptr;
        char* cleanup_ptr = nullptr;
        if (TagProcessing::Open_ready_tag(tag_bytes, file_size, tagdata, cleanup_ptr) != TAG_OBJ_TYPE::material_shader_bytecode_resource) {
            delete[] cleanup_ptr;
            continue; //throw exception("tag wasn't the correct type");
        }

        string filename = ExtractFilename(files[i]);

        _0014_ShaderByteCodeResourceSchema* bytecode_resource = (_0014_ShaderByteCodeResourceSchema*)tagdata;

        // handle the simple types first
        for (int b = 0; b < bytecode_resource->vertex_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(bytecode_resource->vertex_shaders_bytecode_[b], filename, "vertex_shader", b);
        for (int b = 0; b < bytecode_resource->pixel_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(bytecode_resource->pixel_shaders_bytecode_[b], filename, "pixel_shader", b);

        for (int b = 0; b < bytecode_resource->hull_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(bytecode_resource->hull_shaders_bytecode_[b], filename, "hull_shader", b);
        for (int b = 0; b < bytecode_resource->domain_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(bytecode_resource->domain_shaders_bytecode_[b], filename, "domain_shader", b);

        // handle the other cases next
        for (int b = 0; b < bytecode_resource->geometry_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(&bytecode_resource->geometry_shaders_bytecode_[b]->base_compiled_shader, filename, "geometry_shader", b);
        for (int b = 0; b < bytecode_resource->compute_shaders_bytecode_.count; b++)
            handle_compiled_shader_block(&bytecode_resource->compute_shaders_bytecode_[b]->base_compiled_shader, filename, "compute_shader", b);

        delete[] cleanup_ptr;
    }


    system("pause");
}


