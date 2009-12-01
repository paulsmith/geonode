/* Copyright 2009 Paul Smith <paulsmith@pobox.com> */
#include "geonode.h"

/* required functions to initialize GEOS */
// TODO: what to do here?? Is printing to stderr the best thing to do ... ?
void notice_handler(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "NOTICE: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void error_handler(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, const_cast<char *>(fmt), ap);
    Handle<Value> exception = Exception::Error(String::New(buf));
    va_end(ap);
    ThrowException(exception);
}

Geometry::Geometry() : geos_geom_(0) {}

Geometry::Geometry(const char* wkt)
{
    this->FromWKT(wkt);
}

Geometry::~Geometry()
{
    // FIXME we also need to release any char* returned from GEOS functions
    if (geos_geom_ != NULL)
	GEOSGeom_destroy(geos_geom_);
}

void Geometry::Initialize(Handle<Object> target)
{
    HandleScope scope;
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    Handle<ObjectTemplate> obj_template = t->InstanceTemplate();
    obj_template->SetInternalFieldCount(1);
    obj_template->Set(String::NewSymbol("version"), String::New(GEOSversion())); 
    obj_template->SetAccessor(String::NewSymbol("srid"), GetSRID, SetSRID);
    obj_template->SetAccessor(String::NewSymbol("type"), GetType);
    obj_template->SetAccessor(String::NewSymbol("area"), GetArea);
    obj_template->SetAccessor(String::NewSymbol("length"), GetLength);

    NODE_SET_PROTOTYPE_METHOD(t, "fromWkt", FromWKT);
    NODE_SET_PROTOTYPE_METHOD(t, "toWkt", ToWKT);
    // Unary predicates
    NODE_SET_PROTOTYPE_METHOD(t, "isEmpty", IsEmpty);
    NODE_SET_PROTOTYPE_METHOD(t, "isValid", IsValid);
    NODE_SET_PROTOTYPE_METHOD(t, "isSimple", IsSimple);
    NODE_SET_PROTOTYPE_METHOD(t, "isRing", IsRing);
    NODE_SET_PROTOTYPE_METHOD(t, "hasZ", HasZ);
    // Binary predicates
    NODE_SET_PROTOTYPE_METHOD(t, "disjoint", Disjoint);
    NODE_SET_PROTOTYPE_METHOD(t, "touches", Touches);
    NODE_SET_PROTOTYPE_METHOD(t, "intersects", Intersects);
    NODE_SET_PROTOTYPE_METHOD(t, "crosses", Crosses);
    NODE_SET_PROTOTYPE_METHOD(t, "within", Within);
    NODE_SET_PROTOTYPE_METHOD(t, "contains", Contains);
    NODE_SET_PROTOTYPE_METHOD(t, "overlaps", Overlaps);
    NODE_SET_PROTOTYPE_METHOD(t, "equals", Equals);
    // NODE_SET_PROTOTYPE_METHOD(t, "equalsexact", EqualsExact); FIXME

    NODE_SET_PROTOTYPE_METHOD(t, "distance", Distance);

    target->Set(String::NewSymbol("Geometry"), t->GetFunction());
}

bool Geometry::FromWKT(const char* wkt)
{
    GEOSWKTReader *wkt_reader = GEOSWKTReader_create();
    geos_geom_ = GEOSWKTReader_read(wkt_reader, wkt);
    return geos_geom_ == NULL ? false : true;
}

Handle<Value> Geometry::New(const Arguments& args)
{
    Geometry *geom;
    HandleScope scope;
    if (args.Length() == 0) {
	geom = new Geometry();
    } else if (args.Length() == 1 && args[0]->IsString()) {
	String::Utf8Value wkt(args[0]->ToString());
	geom = new Geometry(*wkt);
    }
    geom->Wrap(args.This());
    return args.This();
}

Handle<Value> Geometry::FromWKT(const Arguments& args)
{
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
    HandleScope scope;
    String::AsciiValue wkt(args[0]->ToString());
    bool r = geom->FromWKT(*wkt);
    if (!r) {
	return ThrowException(String::New("invalid WKT"));
    }
    return Undefined();
}

Handle<Value> Geometry::ToWKT(const Arguments& args)
{
    Local<Value> wktjs;
    GEOSWKTWriter *wkt_writer = GEOSWKTWriter_create();
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
    HandleScope scope;
    if (geom->geos_geom_ != NULL) {
	char *wkt = GEOSWKTWriter_write(wkt_writer, geom->geos_geom_);
	wktjs = String::New(wkt);
	GEOSFree(wkt);
    } else {
	wktjs = String::New("");
    }
    return scope.Close(wktjs);
}

GEONODE_GEOS_UNARY_PREDICATE(IsEmpty, isEmpty, GEOSisEmpty);
GEONODE_GEOS_UNARY_PREDICATE(IsValid, isValid, GEOSisValid);
GEONODE_GEOS_UNARY_PREDICATE(IsSimple, isSimple, GEOSisSimple);
GEONODE_GEOS_UNARY_PREDICATE(IsRing, isRing, GEOSisRing);
GEONODE_GEOS_UNARY_PREDICATE(HasZ, hasZ, GEOSHasZ);
GEONODE_GEOS_BINARY_PREDICATE(Disjoint, disjoin, GEOSDisjoint);
GEONODE_GEOS_BINARY_PREDICATE(Touches, touches, GEOSTouches);
GEONODE_GEOS_BINARY_PREDICATE(Intersects, intersects, GEOSIntersects);
GEONODE_GEOS_BINARY_PREDICATE(Crosses, crosses, GEOSCrosses);
GEONODE_GEOS_BINARY_PREDICATE(Within, within, GEOSWithin);
GEONODE_GEOS_BINARY_PREDICATE(Contains, contains, GEOSContains);
GEONODE_GEOS_BINARY_PREDICATE(Overlaps, overlaps, GEOSOverlaps);
GEONODE_GEOS_BINARY_PREDICATE(Equals, equals, GEOSEquals);
// GEONODE_GEOS_BINARY_PREDICATE(EqualsExact, equalsexact, GEOSEqualsExact); FIXME takes tolerance argument

Handle<Value> Geometry::GetSRID(Local<String> name, const AccessorInfo& info)
{
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());
    const int srid = GEOSGetSRID(geom->geos_geom_);
    if (srid == 0)
     	return ThrowException(String::New("couldn't get SRID (maybe it wasn't set)"));
    return Integer::New(srid);
}

void Geometry::SetSRID(Local<String> name, Local<Value> value, const AccessorInfo& info)
{
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());
    GEOSSetSRID(geom->geos_geom_, value->Int32Value());
}

Handle<Value> Geometry::GetType(Local<String> name, const AccessorInfo& info)
{
    HandleScope scope;
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());
    char *type = GEOSGeomType(geom->geos_geom_);
    if (type == NULL)
     	return ThrowException(String::New("couldn't get geometry type"));
    Handle<Value> type_obj = String::New(type);
    GEOSFree(type);
    return scope.Close(type_obj);
}

Handle<Value> Geometry::GetArea(Local<String> name, const AccessorInfo& info)
{
    HandleScope scope;
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());
    double area;
    int r = GEOSArea(geom->geos_geom_, &area);
    if (r != 1)
     	return ThrowException(String::New("couldn't get area"));
    Handle<Value> area_obj = Number::New(area); 
    return scope.Close(area_obj);
}

Handle<Value> Geometry::GetLength(Local<String> name, const AccessorInfo& info)
{
    HandleScope scope;
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(info.Holder());
    double length;
    int r = GEOSLength(geom->geos_geom_, &length);
    if (r != 1)
     	return ThrowException(String::New("couldn't get length"));
    Handle<Value> length_obj = Number::New(length); 
    return scope.Close(length_obj);
}

Handle<Value> Geometry::Distance(const Arguments& args)
{
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
    HandleScope scope;
    if (args.Length() != 1)
	return ThrowException(String::New("missing other geometry"));
    Geometry *other = ObjectWrap::Unwrap<Geometry>(args[0]->ToObject());
    double distance;
    int r = GEOSDistance(geom->geos_geom_, other->geos_geom_, &distance);
    if (r != 1)
     	return ThrowException(String::New("couldn't get distance"));
    Handle<Value> distance_obj = Number::New(distance);
    return scope.Close(distance_obj);
}

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    initGEOS(notice_handler, error_handler);
    Geometry::Initialize(target);
}

// TODO: where to call finishGEOS(); ? Is it even necessary?
