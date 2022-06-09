#include "ExportSymbolInfo.h"

void exportPosition(Token::Position pos, std::ostream& out)
{
	out << "\"file\": \"" << pos.file << "\",";
	out << "\"line\": " << pos.line << ",";
	out << "\"col\": " << pos.column;
}

// Export the symbol info to a text file with the json format.
void exportSymbolInfo(SymbolRef root, std::ostream& out)
{
	out << "{";

	out << "\"name\": \"" << root->name << "\",";

	out << "\"pos\": {";
	exportPosition(root->pos, out);
	out << "},";

	out << "\"type\": \"" << SymTypeToString(root->type) << "\",";

	out << "\"state\": \"" << SymStateToString(root->state) << "\",";

	out << "\"subSymbols\": [";
	{
		auto it = root->subSymbols.begin();
		while (it != root->subSymbols.end())
		{
			exportSymbolInfo(it->second, out);
			if (++it != root->subSymbols.end())
				out << ",";
		}
	}
	out << "]";

	switch (root->type)
	{
	case SymType::Namespace:
		out << ",\"size\": " << root->frame.size;
		out << ",\"totalOffset\": " << root->frame.totalOffset;
		break;
	case SymType::Global:
	case SymType::FunctionName:
		break;
	case SymType::Variable:
		out << ",\"modName\": \"" << root->var.modName << "\"";
		out << ",\"offset\": " << root->var.offset;
		out << ",\"datatype\": \"" << getDatatypeStr(root->var.datatype) << "\"";
		out << ",\"context\": \"" << SymVarContextToString(root->var.context) << "\"";
		break;
	case SymType::FunctionSpec:
	case SymType::ExtFunc:
		out << ",\"retType\": \"" << getDatatypeStr(root->func.retType) << "\"";
		out << ",\"retOffset\": " << root->func.retOffset;
		out << ",\"params\": [";
		for (auto it = root->func.params.begin(); it != root->func.params.end(); )
		{
			exportSymbolInfo(*it, out);
			if (++it != root->func.params.end())
				out << ",";
		}
		out << "]";
		out << ",\"size\": " << root->frame.size;
		out << ",\"totalOffset\": " << root->frame.totalOffset;
		break;
	case SymType::Pack:
		out << ",\"size\": " << root->frame.size;
		out << ",\"isUnion\": " << (root->pack.isUnion ? "true" : "false");
		break;
	case SymType::Enum:
		break;
	case SymType::EnumMember:
		out << ",\"value\": " << root->enumValue;
		break;
	case SymType::Macro:
		out << ",\"macro\": \"";
		for (auto it = root->macroTokens.begin(); it != root->macroTokens.end(); )
		{
			if (it->type == Token::Type::String)
				out << "\\\"" << it->value << "\\\"";
			else
				out << it->value;
			
			if (++it != root->macroTokens.end())
				out << " ";
		}
		out << "\"";
		break;
	}

	// TODO: Export Type specific info.

	out << "}";
}