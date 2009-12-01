/* Copyright 2009 Paul Smith <paulsmith@pobox.com> */
#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <node.h>
#include <node_object_wrap.h>
#include "geos_c.h"

/**
 * A convenience for defining repetitive wrappers of GEOS unary
 * predicate functions. 
 */
#define GEONODE_GEOS_UNARY_PREDICATE(cppmethod, jsmethod, geosfn)	\
    Handle<Value> Geometry::cppmethod(const Arguments& args)		\
    { 									\
	Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());	\
	HandleScope scope;						\
	unsigned char r = geosfn(geom->geos_geom_);			\
	if (r == 2)							\
	    return ThrowException(String::New(#jsmethod"() failed"));	\
	return r ? True() : False();					\
    };

/**
 * A convenience for defining repetitive wrappers of GEOS binary
 * predicate functions. 
 */
#define GEONODE_GEOS_BINARY_PREDICATE(cppmethod, jsmethod, geosfn)	\
    Handle<Value> Geometry::cppmethod(const Arguments& args)		\
    {									\
	Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());	\
	HandleScope scope;						\
	if (args.Length() != 1) {					\
	    return ThrowException(String::New("other geometry required"));	\
	}								\
	Geometry *other = ObjectWrap::Unwrap<Geometry>(args[0]->ToObject());	\
	unsigned char r = geosfn(geom->geos_geom_, other->geos_geom_);	\
	if (r == 2) {							\
	    return ThrowException(String::New(#jsmethod"() failed"));	\
	}								\
	return r ? True() : False();					\
    };

using namespace v8;
using namespace node;

class Geometry : public ObjectWrap {
 public:
    GEOSGeometry *geos_geom_;
    Geometry();
    Geometry(const char* wkt);
    ~Geometry();
    static void Initialize(Handle<Object> target);
    bool FromWKT(const char* wkt);

 protected:
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> FromWKT(const Arguments& args);
    static Handle<Value> ToWKT(const Arguments& args);
    // GEOS unary predicates
    static Handle<Value> IsEmpty(const Arguments& args);
    static Handle<Value> IsValid(const Arguments& args);
    static Handle<Value> IsSimple(const Arguments& args);
    static Handle<Value> IsRing(const Arguments& args);
    static Handle<Value> HasZ(const Arguments& args);
    // GEOS binary predicates
    static Handle<Value> Disjoint(const Arguments& args);
    static Handle<Value> Touches(const Arguments& args);
    static Handle<Value> Intersects(const Arguments& args);
    static Handle<Value> Crosses(const Arguments& args);
    static Handle<Value> Within(const Arguments& args);
    static Handle<Value> Contains(const Arguments& args);
    static Handle<Value> Overlaps(const Arguments& args);
    static Handle<Value> Equals(const Arguments& args);
    // static Handle<Value> EqualsExact(const Arguments& args); FIXME

    // GEOS geometry info
    static Handle<Value> GetSRID(Local<String> name, const AccessorInfo& info);
    static void SetSRID(Local<String> name, Local<Value> value, const AccessorInfo& info);

    // GEOS misc
    static Handle<Value> GetType(Local<String> name, const AccessorInfo& info);
    static Handle<Value> GetArea(Local<String> name, const AccessorInfo& info);
    static Handle<Value> GetLength(Local<String> name, const AccessorInfo& info);
    static Handle<Value> Distance(const Arguments& args);
};
