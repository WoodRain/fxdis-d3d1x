#include "sm4_decompile.h"

namespace sm4 {

// utils
template <typename T>
picojson::value convert_to_json(T value)
{
	return convert_to_json(value, std::is_integral<T>());
}

template <>
picojson::value convert_to_json<char>(char value)
{
	return picojson::value(std::string(1, value));
}

template <>
picojson::value convert_to_json<bool>(bool value)
{
	return picojson::value(value);
}

template <typename T>
picojson::value convert_to_json(T value, std::true_type)
{
	return picojson::value(static_cast<int64_t>(value));
}

template <typename T>
picojson::value convert_to_json(T value, std::false_type)
{
	return picojson::value(value);	
}

// general
picojson::value ast_node::dump()
{
	picojson::object object;
	object["_type"] = picojson::value(this->get_type_string());

	return picojson::value(object);
}

picojson::value super_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();
	picojson::array children;

	for (auto child : this->children)
		children.push_back(child->dump());

	object["children"] = picojson::value(children);

	return picojson::value(object);
}

// variables
template <typename T>
picojson::value variable_node<T>::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["value"] = convert_to_json(this->value);

	return picojson::value(object);
}

// This language is anathema to code cleanliness, quality, organization, or style
template class variable_node<float>;
template class variable_node<int32_t>;
template class variable_node<uint32_t>;
template class variable_node<double>;
template class variable_node<int64_t>;
template class variable_node<uint64_t>;

picojson::value global_index_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["index"] = convert_to_json(this->index);

	return picojson::value(object);
}

picojson::value vector_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();
	picojson::array values;

	for (auto value : this->values)
		values.push_back(value->dump());

	object["values"] = picojson::value(values);

	return picojson::value(object);
}

// index
picojson::value mask_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();
	picojson::array indices;

	for (auto index : this->indices)
		indices.push_back(convert_to_json("xyzw"[index]));

	object["indices"] = picojson::value(indices);
	object["value"] = this->value->dump();

	return picojson::value(object);
}

picojson::value swizzle_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();
	picojson::array indices;

	for (auto index : this->indices)
		indices.push_back(convert_to_json("xyzw"[index]));

	object["indices"] = picojson::value(indices);
	object["value"] = this->value->dump();

	return picojson::value(object);
}

picojson::value scalar_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["index"] = convert_to_json("xyzw"[index]);
	object["value"] = this->value->dump();

	return picojson::value(object);
}

picojson::value index_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["index"] = this->index->dump();
	object["value"] = this->value->dump();

	return picojson::value(object);
}

// comparison
picojson::value comparison_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["not_zero"] = convert_to_json(this->not_zero);
	object["value"] = this->value->dump();

	return picojson::value(object);
}

// n-ary functions
picojson::value unary_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["value"] = this->value->dump();

	return picojson::value(object);
}

picojson::value binary_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["destination"] = this->output->dump();
	object["input"] = this->input->dump();

	return picojson::value(object);
}

picojson::value ternary_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["destination"] = this->output->dump();
	object["lhs"] = this->lhs->dump();
	object["rhs"] = this->rhs->dump();

	return picojson::value(object);
}

picojson::value quaternary_node::dump()
{
	picojson::object object = base_class::dump().get<picojson::object>();

	object["destination"] = this->output->dump();
	object["lhs"] = this->lhs->dump();
	object["rhs1"] = this->rhs1->dump();
	object["rhs2"] = this->rhs2->dump();

	return picojson::value(object);
}

}