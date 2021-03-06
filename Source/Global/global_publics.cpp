// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "../HTTP/httpcall.h"
#include "buildver.h"
#include "global.h"
#include "trace_internal.h"

using namespace xbox::httpclient;

STDAPI 
HCGetLibVersion(_Outptr_ const char** version) HC_NOEXCEPT
try
{
    if (version == nullptr)
    {
        return E_INVALIDARG;
    }

    *version = LIBHTTPCLIENT_VERSION;
    return S_OK;
}
CATCH_RETURN()

STDAPI 
HCInitialize(_In_opt_ HCInitArgs* args) HC_NOEXCEPT
try
{
    HCTraceImplInit();
    return xbox::httpclient::init_http_singleton(args);
}
CATCH_RETURN()

STDAPI_(void) HCCleanup() HC_NOEXCEPT
try
{
    xbox::httpclient::cleanup_http_singleton();
    HCTraceImplCleanup();
}
CATCH_RETURN_WITH(;)

STDAPI_(void)
HCSetHttpCallPerformFunction(
    _In_opt_ HCCallPerformFunction performFunc
    ) HC_NOEXCEPT
{
    auto httpSingleton = get_http_singleton(true);
    if (nullptr == httpSingleton)
        return;

    httpSingleton->m_performFunc = (performFunc == nullptr) ? Internal_HCHttpCallPerformAsync : performFunc;
}

STDAPI 
HCGetHttpCallPerformFunction(
    _Out_ HCCallPerformFunction* performFunc
    ) HC_NOEXCEPT
try
{
    if (performFunc == nullptr)
    {
        return E_INVALIDARG;
    }

    auto httpSingleton = get_http_singleton(true);
    if (nullptr == httpSingleton)
        return E_HC_NOT_INITIALISED;

    *performFunc = httpSingleton->m_performFunc;
    return S_OK;
}
CATCH_RETURN()

STDAPI_(int32_t) HCAddCallRoutedHandler(
    _In_ HCCallRoutedHandler handler,
    _In_ void* context
    ) HC_NOEXCEPT
{
    if (handler == nullptr)
    {
        return -1;
    }

    auto httpSingleton = get_http_singleton(true);
    if (nullptr == httpSingleton)
        return E_HC_NOT_INITIALISED;

    std::lock_guard<std::mutex> lock(httpSingleton->m_callRoutedHandlersLock);
    auto functionContext = httpSingleton->m_callRoutedHandlersContext++;
    httpSingleton->m_callRoutedHandlers[functionContext] = std::make_pair(handler, context);
    return functionContext;
}

STDAPI_(void) HCRemoveCallRoutedHandler(
    _In_ int32_t handlerContext
    ) HC_NOEXCEPT
{
    auto httpSingleton = get_http_singleton(true);
    if (nullptr != httpSingleton)
    {
        std::lock_guard<std::mutex> lock(httpSingleton->m_callRoutedHandlersLock);
        httpSingleton->m_callRoutedHandlers.erase(handlerContext);
    }
}

