/* Copyright 2009 Paul Smith <paulsmith@pobox.com> */
#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <node.h>
#include <node_object_wrap.h>
#include "geos_c.h"

/**
 * A convenience for defining repetitive wrappers of GEOS unary
 * topology functions which return a new geometry.
 */
#define GEONODE_GEOS_UNARY_TOPOLOGY(cppmethod, jsmethod, geosfn)        \
    Handle<Value> Geometry::cppmethod(Local<String> name, const AccessorInfo& info) \
    {									\
	HandleScope scope;						\
	Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());	\
	GEOSGeometry *geos_geom = geosfn(geom->geos_geom_);	\
	if (geos_geom == NULL)						\
	    return ThrowException(String::New("couldn't get "#jsmethod)); \
	Handle<Object> geometry_obj = WrapNewGEOSGeometry(geos_geom);	\
	return scope.Close(geometry_obj);				\
    };

/**
 * A convenience for defining repetitive wrappers of GEOS binary
 * topology functions which return a new geometry.
 */
#define GEONODE_GEOS_BINARY_TOPOLOGY(cppmethod, jsmethod, geosfn)	\
    Handle<Value> Geometry::cppmethod(const Arguments& args)		\
    {									\
	HandleScope scope;						\
	if (args.Length() != 1)						\
	    return ThrowException(String::New("requires other geometry argument")); \
	Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());	\
	Geometry *other = ObjectWrap::Unwrap<Geometry>(args[0]->ToObject()); \
	GEOSGeometry *geos_geom = geosfn(geom->geos_geom_, other->geos_geom_); \
	if (geos_geom == NULL)						\
	    return ThrowException(String::New("couldn't get "#jsmethod)); \
	Handle<Object> geometry_obj = WrapNewGEOSGeometry(geos_geom);	\
	return scope.Close(geometry_obj);				\
    };

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
    Geometry(GEOSGeometry* geom);
    Geometry(const char* wkt);
    ~Geometry();
    static void Initialize(Handle<Object> target);
    bool FromWKT(const char* wkt);

 protected:
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> FromWKT(const Arguments& args);
    static Handle<Value> ToWKT(const Arguments& args);
    // GEOS topology operations
    static Handle<Value> GetEnvelope(Local<String> name, const AccessorInfo& info);
    static Handle<Value> Intersection(const Arguments& args);
    static Handle<Value> Buffer(const Arguments& args);
    static Handle<Value> GetConvexHull(Local<String> name, const AccessorInfo& info);
    static Handle<Value> Difference(const Arguments& args);
    static Handle<Value> SymDifference(const Arguments& args);
    static Handle<Value> GetBoundary(Local<String> name, const AccessorInfo& info);
    static Handle<Value> Union(const Arguments& args);
    static Handle<Value> GetPointOnSurface(Local<String> name, const AccessorInfo& info);
    static Handle<Value> GetCentroid(Local<String> name, const AccessorInfo& info);
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

 private:
    static Persistent<FunctionTemplate> geometry_template_;
    static Handle<FunctionTemplate> MakeGeometryTemplate();
    static Handle<Object> WrapNewGEOSGeometry(GEOSGeometry *geos_geom);
};
