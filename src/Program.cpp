#include "Program.h"

#include <cassert>

#include "Errors/QinpError.h"

std::string getSignatureNoRet(const std::vector<Datatype>& paramTypes)
{
	std::string signature;
	for (const auto& paramType : paramTypes)
		signature += "~" + getDatatypeStr(paramType);
	return signature;
}
std::string getSignatureNoRet(const SymbolRef func)
{
	std::string signature;
	for (const auto& param : func->func.params)
		signature += "~" + getDatatypeStr(param->var.datatype);
	if (func->func.isVariadic)
		signature += "~...";
	return signature;
}
std::string getSignatureNoRet(const Expression* callExpr)
{
	if (callExpr->eType != Expression::ExprType::FunctionCall)
		THROW_QINP_ERROR("Expected function call expression!");

	std::string signature;
	for (const auto& param : callExpr->paramExpr)
		signature += "~" + getDatatypeStr(param->datatype);
	return signature;
}

std::string getSignature(const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	return getDatatypeStr(retType) + "~" + getSignatureNoRet(paramTypes);
}
std::string getSignature(const SymbolRef func)
{
	return getDatatypeStr(func->func.retType) + "~" + getSignatureNoRet(func);
}
std::string getSignature(const Expression* callExpr)
{
	return getDatatypeStr(callExpr->datatype) + "~" + getSignatureNoRet(callExpr);
}

std::string getMangledName(const std::string funcName, const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	return funcName + "$" + getSignature(retType, paramTypes);
}
std::string getMangledName(const std::string& funcName, const Expression* callExpr)
{
	return funcName + "$" + getSignature(callExpr);
}
std::string getMangledName(const std::string& varName, const Datatype& datatype)
{
	return varName + "$" + getDatatypeStr(datatype);
}
std::string getMangledName(SymbolRef symbol)
{
	if (isVariable(symbol))
		return getMangledName(symbol->var.modName, symbol->var.datatype);
	if (isFuncSpec(symbol))
		return SymPathToString(getSymbolPath(nullptr, symbol));
	if (isFuncName(symbol))
		SymPathToString(getSymbolPath(nullptr, symbol));
	if (isEnum(symbol))
		return SymPathToString(getSymbolPath(nullptr, symbol));
	assert(false && "Unhandled symbol type!");
	return "";
}

std::string getReadableName(const std::vector<ExpressionRef>& paramExpr)
{
	std::string paramStr;
	for (int i = 0; i < paramExpr.size(); ++i)
	{
		if (i != 0)
			paramStr += ", ";
		paramStr += getReadableName(paramExpr[i]->datatype);
	}
	return paramStr;
}

std::string getReadableName(const std::vector<SymbolRef>& paramSym, bool isVariadic)
{
	std::string paramStr;
	for (int i = 0; i < paramSym.size(); ++i)
	{
		if (i != 0)
			paramStr += ", ";
		paramStr += getReadableName(paramSym[i]->var.datatype);
	}
	if (isVariadic)
		paramStr += paramSym.empty() ? "..." : ", ...";
	return paramStr;
}

std::string getReadableName(SymbolRef symbol)
{
	if (isVariable(symbol))
		return getReadableName(symbol->var.datatype) + " " + symbol->name;
	if (isFuncSpec(symbol))
		return getReadableName(symbol->func.retType) + " " + SymPathToString(getSymbolPath(nullptr, getParent(symbol))) + "(" + getReadableName(symbol->func.params, symbol->func.isVariadic) + ")";
	if (isFuncName(symbol))
		return SymPathToString(getSymbolPath(nullptr, symbol));
	if (isEnum(symbol))
		return SymPathToString(getSymbolPath(nullptr, symbol));
	assert(false && "Unhandled symbol type!");
	return "";
}

std::string getLiteralStringName(int strID)
{
	return "__#str_" + std::to_string(strID);
}
std::string getStaticLocalInitName(int initID)
{
	return "__#stck_" + std::to_string(initID);
}

bool isPackType(const ProgramRef program, const std::string& name)
{
	return isPack(getSymbol(currSym(program), name));
}

bool isPackType(const ProgramRef program, const Datatype& datatype)
{
	return isOfType(datatype, DTType::Name) && isPackType(program, datatype.name);
}

int getPackSize(const ProgramRef program, const std::string& packName)
{
	auto sym = getSymbolFromPath(program->symbols, SymPathFromString(packName));
	if (!isPack(sym))
		return -1;

	if (!isDefined(sym))
		return 0;

	return sym->pack.size;
}

int getDatatypeSize(const ProgramRef program, const Datatype& datatype, bool treatArrayAsPointer)
{
	if (isNull(datatype))
		return sizeof(void*);

	if (isPointer(datatype) || (treatArrayAsPointer && isArray(datatype)))
		return sizeof(void*);

	if (isEnum(program, datatype))
		return sizeof(int64_t);
	
	if (isArray(datatype))
	{
		int elemSize = getDatatypeSize(program, *datatype.subType);
		return elemSize * getDatatypeNumElements(datatype);
	}

	int size = getBuiltinTypeSize(datatype.name);
	if (size >= 0)
		return size;

	size = getPackSize(program, datatype.name);
	if (size >= 0)
		return size;

	return -1;
}

int getDatatypePushSize(const ProgramRef program, const Datatype& datatype)
{
	int size = getDatatypeSize(program, datatype);
	if (size < 0)
		return -1;
	// Round the size to the next multiple of 8
	return (size + 7) & -8;
}

int getDatatypePointedToSize(const ProgramRef program, Datatype datatype)
{
	if (!isPointer(datatype) && !isArray(datatype))
		THROW_QINP_ERROR("Cannot get size of non-pointer datatype");

	dereferenceDatatype(datatype);
	return getDatatypeSize(program, datatype);
}

SymbolRef currSym(const ProgramRef program)
{
	return program->symStack.top();
}