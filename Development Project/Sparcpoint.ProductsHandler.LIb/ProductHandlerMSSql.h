#pragma once

#include "IProductHandler.h"
#include "json.hpp"

namespace iti {
    class ProductHandlerMSSql : public IProductHandler {
      public: 
        ProductHandlerMSSql() : logger(nullptr), state(State::Uninitialized) {}
        ProductHandlerMSSql(const ProductHandlerMSSql &) = delete;
        ProductHandlerMSSql& operator=(const ProductHandlerMSSql &) = delete;
        
        ErrorCode Init(const std::wstring &configJson,
                       ILogger *logger) override;
        ErrorCode Shutdown()            override;

        ErrorCode AddProductDefinition(const std::wstring &name,
                                               const std::wstring &gen_details,
                                               const StrList &categories,
                                               const StrList &metadata,
                                               uint64_t &o_id) override;
        ErrorCode
        GetProductDefinitionById(uint64_t id,
                                 std::wstring &o_prodDefJson) const override;
        ErrorCode GetProductDefinitions(
            const std::wstring &name, const std::wstring &gen_details_regex,
            const StrList &categories, const StrList &metadata,
            int numItemsToGet, std::wstring &o_prodDefJson,
            Handle *o_CollectionHandle = nullptr) const override;
        /* GetNextProductDefinitions Output JSON format: same as for
         * GetProductDefinitions
         * */
        ErrorCode
        GetNextProductDefinitions(Handle collectionHandle, int numItemsToGet,
                                  std::wstring &o_prodDefJson) const override;
        ErrorCode
        CloseCollectionHandle(Handle collectionHandle) override;
        ErrorCode AddProductInventory(uint64_t id,
                            uint64_t numToAdd, uint64_t &o_numPresent) override;
        ErrorCode RemoveProductInventory(uint64_t id,
                                         uint64_t numToRemove,
                                         uint64_t &o_numRemoved,
                                         uint64_t &o_numPresent) override;
        ErrorCode
        ReportProductInventory(uint64_t id, uint64_t &o_numPresent) const override;

      private:
        ILogger *logger;
        enum class State {
            Initialized,
            Uninitialized
        };
        State state;
        nlohmann::json configJson;
    };
    }; // namespace iti
