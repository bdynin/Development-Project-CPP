#pragma once

#include "ProductHandlerMSSql.h"
#include "ProductUtil.h"

using nlohmann::json;

namespace iti {
IProductHandler::ErrorCode
ProductHandlerMSSql::Init(const std::wstring &configJson, ILogger *logger) {
    if ( state == State::Uninitialized)
    {
        this->logger = logger;
        std::string configJsonA = WstrToStr(configJson);
        try {
            this->configJson = json::parse(configJsonA);
        } catch (json::parse_error & /* ex */) {
            return ErrorCode::INVALID_INPUT_PARAM;
        }
        state = State::Initialized;
        return ErrorCode::SUCCESS;
    }
    return ErrorCode::INCORRECT_STATE;
}

IProductHandler::ErrorCode
    ProductHandlerMSSql::Shutdown()
{
    if (state == State::Initialized) {
        this->logger = nullptr;
        state        = State::Uninitialized;
        return ErrorCode::SUCCESS;
    }

    return ErrorCode::INCORRECT_STATE;
}

IProductHandler::ErrorCode ProductHandlerMSSql::AddProductDefinition(
    const std::wstring &name, const std::wstring &gen_details,
    const StrList &categories, const StrList &metadata, uint64_t &o_id) 
{
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode ProductHandlerMSSql::GetProductDefinitionById(
    uint64_t id, std::wstring &o_prodDefJson) const 
{
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode ProductHandlerMSSql::GetProductDefinitions(
    const std::wstring &name, const std::wstring &gen_details_regex,
    const StrList &categories, const StrList &metadata, int numItemsToGet,
    std::wstring &o_prodDefJson, Handle *o_CollectionHandle) const 
{
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode ProductHandlerMSSql::GetNextProductDefinitions(
    Handle collectionHandle, int numItemsToGet, std::wstring &o_prodDefJson) const {
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode
ProductHandlerMSSql::CloseCollectionHandle(Handle collectionHandle) {
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode
ProductHandlerMSSql::AddProductInventory(uint64_t id, uint64_t numToAdd, uint64_t &o_numPresent) {
    return ErrorCode::NOT_IMPLEMENTED;
}


IProductHandler::ErrorCode ProductHandlerMSSql::RemoveProductInventory(uint64_t id, uint64_t numToRemove,
                                            uint64_t &o_numRemoved, uint64_t &o_numPresent) {
    return ErrorCode::NOT_IMPLEMENTED;
}

IProductHandler::ErrorCode
ProductHandlerMSSql::ReportProductInventory(uint64_t id, uint64_t &o_numPresent) const
{
    return ErrorCode::NOT_IMPLEMENTED;
}
}; // namespace iti
