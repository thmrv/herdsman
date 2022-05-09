#include "nativevpncli.h"
#include <napi.h>

void AddVPNLink(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string ConnName = info[0].ToString().Utf8Value();
    const wchar_t* szName = reinterpret_cast<const wchar_t *>(ConnName.c_str());
    std::string ConnServer = info[1].ToString().Utf8Value();
    const wchar_t* szServer = reinterpret_cast<const wchar_t *>(ConnServer.c_str());
    std::string ConnUsername = info[2].ToString().Utf8Value();
    const wchar_t* szUsername = reinterpret_cast<const wchar_t *>(ConnUsername.c_str());
    std::string ConnPassword = info[3].ToString().Utf8Value();
    const wchar_t* szPassword = reinterpret_cast<const wchar_t *>(ConnPassword.c_str());
    VPNCli::AddVPNConn(const_cast<wchar_t *>(szName), const_cast<wchar_t *>(szServer), const_cast<wchar_t *>(szUsername), const_cast<wchar_t *>(szPassword));
}

void StartVPNLink(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string ConnName = info[0].ToString().Utf8Value();
    const wchar_t* szName = reinterpret_cast<const wchar_t *>(ConnName.c_str());
    VPNCli::ExecVPNConn(const_cast<wchar_t *>(szName));
}

void StopVPNLink(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string ConnName = info[0].ToString().Utf8Value();
    const wchar_t* szName = reinterpret_cast<const wchar_t *>(ConnName.c_str());
    VPNCli::CloseVPNConn(const_cast<wchar_t *>(szName));
}

void GetRasDevices(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    VPNCli::PrintDevices();
}

void DestroyVPNLink(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string ConnName = info[0].ToString().Utf8Value();
    const wchar_t* szName = reinterpret_cast<const wchar_t *>(ConnName.c_str());
    VPNCli::RemoveVPNConn(const_cast<wchar_t *>(szName));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "AddVPNLink"),Napi::Function::New(env, AddVPNLink));
    exports.Set(Napi::String::New(env, "StartVPNLink"),Napi::Function::New(env, StartVPNLink));
    exports.Set(Napi::String::New(env, "StopVPNLink"),Napi::Function::New(env, StopVPNLink));
    exports.Set(Napi::String::New(env, "DestroyVPNLink"),Napi::Function::New(env, DestroyVPNLink));
    exports.Set(Napi::String::New(env, "GetRasDevices"),Napi::Function::New(env, GetRasDevices));
    return exports;
}

NODE_API_MODULE(nativevpncli, Init);