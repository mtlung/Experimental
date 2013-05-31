#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <typeinfo>
#include <list>
#include <algorithm>
#include <assert.h>

using namespace std;

void _extract() {}

template<class T1, class... Ts>
void _extract(T1 t1, Ts... ts)
{
	cout << typeid(t1).name() << endl;
	_extract(ts...);
}

template<class... T>
void extract(T... t)
{
	_extract(t...);
}

template<class... T>
void fun(T... t)
{
	array<int, sizeof...(t)> list = {t...};
	cout << list[0];
}

struct Type;

struct Field
{
	void* getPtr(void* self);

	template<class T>
	T* getPtr(void* self);

	std::string name;
	Type* type;
	unsigned offset;
	bool isPointer;
};

struct Func
{
	std::string name;
};

struct Type
{
	Field* getField(const char* name);

	std::string name;
	Type* parent;
	const std::type_info& typeInfo;
	std::vector<Field> fields;
	std::vector<Func> funcs;
};

template<class T> struct Klass;

struct Reflection
{
	template<class T>
	Klass<T> Class(const char* name);

	template<class T, class P>
	Klass<T> Class(const char* name);

	template<class T>
	Type* getType();

	std::list<Type> types;
};

static Reflection reflection;

Field* Type::getField(const char* name)
{
	for(auto i = fields.begin(); i != fields.end(); ++i)
		if(i->name == name)
			return &(*i);
}

void* Field::getPtr(void* self)
{
	return ((char*)self) + offset;
}

template<class T>
T* Field::getPtr(void* self)
{
	if(!type || typeid(T) != type->typeInfo)
		return NULL;
	return (T*)(((char*)self) + offset);
}

template<class T, class C>
Type* ExtractMemberType(T C::*m)
{
	return reflection.getType<T>();
};

template<class T>
struct Klass
{
	template<class F>
	Klass& field(const char* name, F f)
	{
		unsigned offset = reinterpret_cast<unsigned>(&((T*)(NULL)->*f));
		Field tmp = { name, ExtractMemberType(f), offset, false };
		type.fields.push_back(tmp);	// TODO: fields better be sorted according to the offset
		return *this;
	}
	Type& type;
};

template<class T>
Klass<T> Reflection::Class(const char* name)
{
	Type type = { name, NULL, typeid(T) };
	types.push_back(type);
	Klass<T> k = { types.back() };
	return k;
}

template<class T, class P>
Klass<T> Reflection::Class(const char* name)
{
	Type* parent = getType<P>();
	assert(parent);
	Type type = { name, parent, typeid(T), parent->fields, parent->funcs };
	types.push_back(type);
	Klass<T> k = { types.back() };
	return k;
}

template<class T>
Type* Reflection::getType()
{
	auto& types = reflection.types;
	for(auto i = types.begin(); i != types.end(); ++i)
		if(i->typeInfo == typeid(T))
			return &*i;
	return NULL;
}

struct Shape
{
	float area;
};

struct Circle : public Shape
{
	float radius;
};

struct Vector3
{
	float x, y, z;
};

struct Body
{
	Vector3 position;
	Vector3 velocity;
};

int main()
{
//	fun(1);

//	extract(1, 2.3, 3.4f, "hello");

	reflection.Class<bool>("bool");
	reflection.Class<int>("int");
	reflection.Class<float>("float");
	reflection.Class<double>("double");

	reflection
		.Class<Vector3>("Vector3")
		.field("x", &Vector3::x)
		.field("y", &Vector3::y)
		.field("z", &Vector3::z);

	reflection
		.Class<Shape>("Shape")
		.field("area", &Shape::area);

	reflection
		.Class<Circle, Shape>("Circle")
		.field("radius", &Circle::radius);

	reflection
		.Class<Body>("Body")
		.field("position", &Body::position)
		.field("velocity", &Body::velocity);

	{
		Type* t = reflection.getType<Vector3>();
		Vector3 v = { 1, 2, 3 };
		cout << *t->getField("x")->getPtr<float>(&v);
		cout << *t->getField("y")->getPtr<float>(&v);
	}

	{
		Type* t = reflection.getType<Circle>();
		Circle c;
		c.area = 10;
		c.radius = 2;

		cout << *t->getField("area")->getPtr<float>(&c);
		cout << *t->getField("radius")->getPtr<float>(&c);
	}

	return 0;
}
