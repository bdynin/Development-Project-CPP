#include "IProductHandler.h"
#include "ProductHandlerMSSql.h"

namespace iti {
	IProductHandler *ProductHandlerFactory::Create(ProductHandlerType type)
	{
	if (type == ProductHandlerType::MSSQL)
		return new ProductHandlerMSSql();
	return nullptr;
	}
}
