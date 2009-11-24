#include <iostream>
#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <node.h>
#include <node_object_wrap.h>
#include "geos_c.h"

using namespace v8;
using namespace node;
using namespace std;

/* notice_handler & error_handler -- required functions to initialize GEOS */

// TODO: Change this to print a JavaScript string
void
notice_handler(const char *fmt, ...) 
{
    va_list ap;
    fprintf(stdout, "NOTICE: ");
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
}

// TODO: Make this a JavaScript exception
void
error_handler(const char *fmt, ...) 
{
    va_list ap;
    fprintf(stderr, "ERROR: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

class Geometry : public ObjectWrap {
public:
    //    GEOSGeometry _geos_geom;
    Geometry()
    {
	initGEOS(notice_handler, error_handler);
    }

    ~Geometry()
    {
	finishGEOS();
    }

    static void Initialize(Handle<Object> target)
    {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);

	Handle<ObjectTemplate> obj_tpl = t->InstanceTemplate();
	obj_tpl->SetInternalFieldCount(1);
	obj_tpl->Set(String::NewSymbol("version"), String::New(GEOSversion()));

	target->Set(String::NewSymbol("Geometry"), t->GetFunction());
    }

protected:
    static Handle<Value> New(const Arguments& args)
    {
	HandleScope scope;
	
	Geometry *geom = new Geometry();
	geom->Wrap(args.This());
	
	return args.This();
    }
};

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    Geometry::Initialize(target);
}
