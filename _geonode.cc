#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <node.h>
#include <node_object_wrap.h>
#include "geos_c.h"

using namespace v8;
using namespace node;

/* notice_handler & error_handler -- required functions to initialize GEOS */
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

class Geometry : public ObjectWrap {
public:
    GEOSGeometry *geos_geom_;

    Geometry() : geos_geom_(0) {}

    Geometry(const char* wkt)
    {
	this->FromWKT(wkt);
    }

    ~Geometry()
    {
	// FIXME we also need to release any char* returned from GEOS functions
	if (geos_geom_ != NULL)
	    GEOSGeom_destroy(geos_geom_);
    }

    static void Initialize(Handle<Object> target)
    {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);

	Handle<ObjectTemplate> obj_tpl = t->InstanceTemplate();
	obj_tpl->SetInternalFieldCount(1);
	obj_tpl->Set(String::NewSymbol("version"), String::New(GEOSversion()));

	NODE_SET_PROTOTYPE_METHOD(t, "fromWkt", FromWKT);
	NODE_SET_PROTOTYPE_METHOD(t, "toWkt", ToWKT);
	NODE_SET_PROTOTYPE_METHOD(t, "contains", Contains);

	target->Set(String::NewSymbol("Geometry"), t->GetFunction());
    }

    bool FromWKT(const char* wkt)
    {
	GEOSWKTReader *wkt_reader = GEOSWKTReader_create();

	geos_geom_ = GEOSWKTReader_read(wkt_reader, wkt);

	return geos_geom_ == NULL ? false : true;
    }

protected:
    static Handle<Value> New(const Arguments& args)
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

    static Handle<Value> FromWKT(const Arguments& args)
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

    static Handle<Value> ToWKT(const Arguments& args)
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

    static Handle<Value> Contains(const Arguments& args)
    {
	Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());

	HandleScope scope;

	if (args.Length() != 1) {
	    return ThrowException(String::New("other geometry required"));
	}

	// // FIXME handle invalid type
	Geometry *other = ObjectWrap::Unwrap<Geometry>(args[0]->ToObject());

	unsigned char r = GEOSContains(geom->geos_geom_, other->geos_geom_);
	if (r == 2) {
	    return ThrowException(String::New("predicate contains() failed"));
	}
	return r ? True() : False();
    }
};

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    initGEOS(notice_handler, error_handler);
    Geometry::Initialize(target);
}

// TODO: where to call finishGEOS(); ? Is it even necessary?
