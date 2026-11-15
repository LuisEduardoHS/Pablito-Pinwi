#ifndef GC_SHADER_COMPILER_H
#define GC_SHADER_COMPILER_H

#include <d3dcompiler.h>
#include <fstream>
#include <new>
#pragma comment(lib, "D3DCompiler.lib")

// June 2010's compiler predates D3DCompileFromFile and its standard include handler.
// All scene shaders/includes live together in the project's working directory.
class SceneShaderInclude : public ID3DInclude
{
public:
    HRESULT __stdcall Open(D3D_INCLUDE_TYPE, LPCSTR filename, LPCVOID, LPCVOID* contents, UINT* bytes)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) return E_FAIL;
        const std::streamoff length = file.tellg();
        if (length <= 0 || length > 4 * 1024 * 1024) return E_FAIL;
        char* data = new (std::nothrow) char[static_cast<size_t>(length)];
        if (!data) return E_OUTOFMEMORY;
        file.seekg(0);
        if (!file.read(data, length)) { delete[] data; return E_FAIL; }
        *contents = data;
        *bytes = static_cast<UINT>(length);
        return S_OK;
    }
    HRESULT __stdcall Close(LPCVOID contents)
    {
        delete[] static_cast<const char*>(contents);
        return S_OK;
    }
};

inline bool CompileSceneShader(LPCWSTR path, const char* entry, const char* profile, ID3DBlob** code)
{
    ID3DBlob* errors = NULL;
    SceneShaderInclude includes;
    const HRESULT result = D3DX11CompileFromFile(path, NULL, &includes,
        entry, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, code, &errors, NULL);
    if (FAILED(result))
    {
        if (errors)
            MessageBoxA(NULL, static_cast<const char*>(errors->GetBufferPointer()),
                "Error al compilar shader", MB_OK | MB_ICONERROR);
        else
            MessageBoxW(NULL, path, L"No se pudo abrir el shader", MB_OK | MB_ICONERROR);
    }
    if (errors) errors->Release();
    return SUCCEEDED(result);
}

#endif
