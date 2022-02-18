#pragma once

#include <string>
#include <vector>

// Assume that iti namespace implements ILogger interface
namespace iti {
namespace logger {
class ILogger;
}
} // namespace iti

namespace iti {
enum class ProductHandlerType { MSSQL };
using ILogger = iti::logger::ILogger;

class IProductHandler;

// Class factory
class ProductHandlerFactory {
  public:
    ProductHandlerFactory(ILogger *logger = nullptr)
        : logger(logger) {}
    ProductHandlerFactory(const ProductHandlerFactory &) = delete;
    ProductHandlerFactory& operator=(const ProductHandlerFactory &) = delete;

    IProductHandler *Create(ProductHandlerType type);
  private:
    ILogger *logger;
};

// Interface
class IProductHandler {
  public:
    enum class ErrorCode : int {
        SUCCESS,
        INVALID_INPUT_PARAM,
        NOT_FOUND,
        RESOURCE_UNAVAILABLE,
        PARTIAL_RESOURCE_UNAVAILABLE,
        NOT_READY,
        INTERNAL_ERROR,
        NOT_IMPLEMENTED,
        INCORRECT_STATE
    };
    using StrList = std::vector<std::wstring>;
    using Handle = void*;

    // configJson params are implementation-specific
    /* For SQL Server config JSON format:
     * {"ConnStr" : "some-ODBC-conn-str"}
     */
    virtual ErrorCode Init(const std::wstring &configJson, ILogger *logger) = 0;
    virtual ErrorCode Shutdown() = 0;

    virtual ErrorCode AddProductDefinition(const std::wstring &name,
                                           const std::wstring &gen_details,
                                           const StrList &categories,
                                           const StrList &metadata,
                                           uint64_t& o_id) = 0;
    /* GetProductDefinitionById Output JSON format:
    * {
    *	"id" : <string>,
    *	"name" : <string>,
    *	"categories" : [<string>,...,<string>],
    *	"metadata" : [<string>,...,<string>],
    *	"general-details" : <string>
    * }
    */
    virtual ErrorCode
    GetProductDefinitionById(uint64_t id,
                             std::wstring &o_prodDefJson) const = 0;
    /* GetProductDefinitions Output JSON format:
     * {
     *	[
     *	{
     *	"id" : <string>,
     *	"name" : <string>,
     *	"categories" : [<string>,...,<string>],
     *	"metadata" : [<string>,...,<string>],
     *	"general-details" : <string>
     *	},
     *	...
     *	]
     * }
     */
    virtual ErrorCode
    GetProductDefinitions(const std::wstring &name, const std::wstring& gen_details_regex, const StrList &categories,
                          const StrList &metadata, int numItemsToGet,
                          std::wstring &o_prodDefJson, Handle *o_CollectionHandle = nullptr) const = 0;
    /* GetNextProductDefinitions Output JSON format: same as for GetProductDefinitions
    * */
    virtual ErrorCode
    GetNextProductDefinitions(Handle collectionHandle, int numItemsToGet,
                              std::wstring &o_prodDefJson) const      = 0;
    virtual ErrorCode
    CloseCollectionHandle(Handle collectionHandle)           = 0;
    virtual ErrorCode AddProductInventory(uint64_t id, uint64_t numToAdd,
                                          uint64_t &o_numPresent)          = 0;
    virtual ErrorCode RemoveProductInventory(uint64_t id, uint64_t numToRemove,
                                             uint64_t &o_numRemoved,
                                             uint64_t &o_numPresent)      = 0;
    virtual ErrorCode ReportProductInventory(uint64_t id,
                                             uint64_t& o_numPresent) const = 0;
  protected:
    virtual ~IProductHandler() {}
};
} // namespace iti
