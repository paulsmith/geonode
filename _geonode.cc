/* Copyright 2009 Paul Smith <paulsmith@pobox.com> */
#include "geonode.h"
#include <string.h>

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

Geometry::Geometry(GEOSGeometry* geom) : geos_geom_(geom) {}

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

Persistent<FunctionTemplate> Geometry::geometry_template_;

Handle<Object> Geometry::WrapNewGEOSGeometry(GEOSGeometry *geos_geom)
{
    HandleScope scope;
    Local<Object> geom_obj = geometry_template_->InstanceTemplate()->NewInstance();
    Geometry *geom = new Geometry(geos_geom);
    geom->Wrap(geom_obj);
    return scope.Close(geom_obj);
}

Handle<FunctionTemplate> Geometry::MakeGeometryTemplate()
{
    HandleScope scope;
    Handle<FunctionTemplate> t = FunctionTemplate::New(New);
    Local<ObjectTemplate> obj_t = t->InstanceTemplate();
    obj_t->SetInternalFieldCount(1);
    obj_t->Set(String::NewSymbol("_geosVersion"), String::New(GEOSversion()));
    obj_t->SetAccessor(String::NewSymbol("envelope"), GetEnvelope);
    obj_t->SetAccessor(String::NewSymbol("convexHull"), GetConvexHull);
    obj_t->SetAccessor(String::NewSymbol("boundary"), GetBoundary);
    obj_t->SetAccessor(String::NewSymbol("pointOnSurface"), GetPointOnSurface);
    obj_t->SetAccessor(String::NewSymbol("centroid"), GetCentroid);
    obj_t->SetAccessor(String::NewSymbol("srid"), GetSRID, SetSRID);
    obj_t->SetAccessor(String::NewSymbol("type"), GetType);
    obj_t->SetAccessor(String::NewSymbol("area"), GetArea);
    obj_t->SetAccessor(String::NewSymbol("length"), GetLength);
    return scope.Close(t);
}

void Geometry::Initialize(Handle<Object> target)
{
    HandleScope scope;
    if (geometry_template_.IsEmpty())
	geometry_template_ = Persistent<FunctionTemplate>::New(MakeGeometryTemplate());
    Handle<FunctionTemplate> t = geometry_template_;
    NODE_SET_PROTOTYPE_METHOD(t, "fromWkt", FromWKT);
    NODE_SET_PROTOTYPE_METHOD(t, "toWkt", ToWKT);
    // Topology operations
    NODE_SET_PROTOTYPE_METHOD(t, "intersection", Intersection);
    NODE_SET_PROTOTYPE_METHOD(t, "buffer", Buffer);
    NODE_SET_PROTOTYPE_METHOD(t, "difference", Difference);
    NODE_SET_PROTOTYPE_METHOD(t, "symDifference", SymDifference);
    NODE_SET_PROTOTYPE_METHOD(t, "union", Union);
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
GEONODE_GEOS_UNARY_TOPOLOGY(GetEnvelope, envelope, GEOSEnvelope);
GEONODE_GEOS_UNARY_TOPOLOGY(GetConvexHull, convexHull, GEOSConvexHull);
GEONODE_GEOS_UNARY_TOPOLOGY(GetBoundary, boundary, GEOSBoundary);
GEONODE_GEOS_UNARY_TOPOLOGY(GetPointOnSurface, pointOnSurface, GEOSPointOnSurface);
GEONODE_GEOS_UNARY_TOPOLOGY(GetCentroid, centroid, GEOSGetCentroid);
GEONODE_GEOS_BINARY_TOPOLOGY(Intersection, intersection, GEOSIntersection);
GEONODE_GEOS_BINARY_TOPOLOGY(Difference, difference, GEOSDifference);
GEONODE_GEOS_BINARY_TOPOLOGY(SymDifference, symDifference, GEOSSymDifference);
GEONODE_GEOS_BINARY_TOPOLOGY(Union, union, GEOSUnion);

Handle<Value> Geometry::Buffer(const Arguments& args)
{
    HandleScope scope;
    double width;
    int quadsegs = 8;
    if (args.Length() < 1)
	return ThrowException(String::New("requires width argument"));
    if (!args[0]->IsNumber())
	return ThrowException(Exception::TypeError(String::New("width argument must be a number")));
    Geometry *geom = ObjectWrap::Unwrap<Geometry>(args.This());
    width = args[0]->NumberValue();
    if (args.Length() == 2)
	quadsegs = args[1]->Int32Value();
    GEOSGeometry *buffer = GEOSBuffer(geom->geos_geom_, width, quadsegs);
    if (buffer == NULL)
	return ThrowException(String::New("couldn't buffer geometry"));
    Handle<Object> buffer_obj = WrapNewGEOSGeometry(buffer);
    return scope.Close(buffer_obj);
}

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

GEOSGeometry *Geometry::GetGEOSGeometry()
{
    return this->geos_geom_;
}

GEOSCoordSequence *Geometry::ApplyPointTransformationToCoordSequence(PointTransformer *t, const GEOSCoordSequence *seq)
{
    GEOSCoordSequence   *ret = GEOSCoordSeq_clone(seq);
    unsigned int        sz;
    
    GEOSCoordSeq_getSize(ret, &sz);

    for (unsigned int i = 0; i < sz; i++) {
        double x, y;

        GEOSCoordSeq_getX(ret, i, &x);
        GEOSCoordSeq_getY(ret, i, &y);

        t->Transform(&x, &y, NULL);

        GEOSCoordSeq_setX(ret, i, x);
        GEOSCoordSeq_setY(ret, i, y);
    }    
    
    return ret;
}

GEOSGeometry *Geometry::ApplyPointTransformationToSingleGeometry(PointTransformer *t, const GEOSGeometry *g)
{
    int             gtype   = GEOSGeomTypeId(g);
    GEOSGeometry    *ng     = NULL;
    
    if (gtype == GEOS_POINT || gtype == GEOS_LINESTRING || gtype == GEOS_LINEARRING) {
        const GEOSCoordSequence *seq =  GEOSGeom_getCoordSeq(g);
        GEOSCoordSequence       *nseq = ApplyPointTransformationToCoordSequence(t, seq);
        
        // this is silly -- GEOS really needs a shortcut for this
        if (gtype == GEOS_POINT) {
            ng = GEOSGeom_createPoint(nseq);
        }
        if (gtype == GEOS_LINESTRING) {
            ng = GEOSGeom_createLineString(nseq);
        }
        if (gtype == GEOS_LINEARRING) {
            ng = GEOSGeom_createLinearRing(nseq);
        }
    }
    else if (gtype == GEOS_POLYGON) {
        int                 ircnt   = GEOSGetNumInteriorRings(g);
        const GEOSGeometry  *ext    = GEOSGetExteriorRing(g);
        GEOSGeometry        *next   = ApplyPointTransformationToSingleGeometry(t, ext);
        GEOSGeometry        **rings = NULL;
        
        if (ircnt > 0) {
            // This shares a lot in common with the code below in ApplyPointTransformation,
            // refactor into a single method?
            rings = new GEOSGeometry *[ircnt];
            
            for (int i = 0; i < ircnt; i++) {
                rings[i] = ApplyPointTransformationToSingleGeometry(t, GEOSGetInteriorRingN(g, i));
            }
        }
        
        ng = GEOSGeom_createPolygon(next, rings, ircnt);
        
        if (rings) {
            delete rings;
        }
    }
    
    return ng;
}

void Geometry::ApplyPointTransformation(PointTransformer *t)
{
    GEOSGeometry    *g      = this->geos_geom_;
    GEOSGeometry    *ng     = NULL;
    int             gtype   = GEOSGeomTypeId(g);
    int             gcount  = GEOSGetNumGeometries(g);
    
    if (gcount == 1) {
        ng = ApplyPointTransformationToSingleGeometry(t, g);
    }
    else {
        GEOSGeometry **coll = new GEOSGeometry *[gcount];
    
        try {
            for (int i = 0; i < gcount; i++) {
                coll[i] = ApplyPointTransformationToSingleGeometry(t, GEOSGetGeometryN(g, i));
            }
        }
        catch (TransformerException ex) {
            // free up our memory before we pass this up.
            delete coll;
            throw ex;
        }

        ng = GEOSGeom_createCollection(gtype, coll, gcount);
        delete coll;
    }
    
    if (ng != NULL) {
        GEOSGeom_destroy(this->geos_geom_);
        this->geos_geom_ = ng;
    }
}

TransformerException::TransformerException(char *description) {
    strncpy(this->description, description, 1024);
}
char *TransformerException::GetDescription() { return this->description; }

Projection::Projection(const char* init)
{
    this->pj = pj_init_plus(init);
}

Projection::~Projection()
{
    pj_free(this->pj);
}

bool Projection::IsValid()
{
    return (this->pj != NULL);
}

Persistent<FunctionTemplate> Projection::projection_template_;

Handle<FunctionTemplate> Projection::MakeProjectionTemplate()
{
    HandleScope scope;
    Handle<FunctionTemplate> t = FunctionTemplate::New(New);
    
    // Setup "Static" Members
    t->Set(String::NewSymbol("transform"), FunctionTemplate::New(Transform));
    
    // Setup Instance Members
    Local<ObjectTemplate> obj_t = t->InstanceTemplate();
    obj_t->SetInternalFieldCount(1);
    obj_t->Set(String::NewSymbol("__projVersion"), String::New(pj_get_release()));
    obj_t->SetAccessor(String::NewSymbol("definition"), GetDefinition);

    return scope.Close(t);
}

void Projection::Initialize(Handle<Object> target)
{
    HandleScope scope;
    if (projection_template_.IsEmpty())
	    projection_template_ = Persistent<FunctionTemplate>::New(MakeProjectionTemplate());
    Handle<FunctionTemplate> t = projection_template_;
    target->Set(String::NewSymbol("Projection"), t->GetFunction());
}

Handle<Value> Projection::New(const Arguments& args)
{
    Projection      *proj;
    HandleScope     scope;
    
    if (args.Length() == 1 && args[0]->IsString()) {
        String::Utf8Value init(args[0]->ToString());
        proj = new Projection(*init);
        
        if (!proj->IsValid()) {
            int     *errno          = pj_get_errno_ref();
            char    *description    = pj_strerrno(*errno);
            
            return ThrowException(String::New(description));
        }
    }
    else {
        return ThrowException(String::New("No valid arguments passed for projection initialization string."));
    }
    
    proj->Wrap(args.This());
    return args.This();        
}

Handle<Value> Projection::GetDefinition(Local<String> name, const AccessorInfo& info)
{
    HandleScope     scope;
    Projection      *self = ObjectWrap::Unwrap<Projection>(info.Holder());
    char            *def;
    Handle<Value>   def_obj;
    
    def = pj_get_def(self->pj, 0);
    def_obj = String::New(def);
    
    pj_dalloc(def);
    return scope.Close(def_obj);
}

Handle<Value> Projection::Transform(const Arguments& args)
{
    HandleScope     scope;
    Handle<Value>   geom_obj;
    
    if (args.Length() < 3) {
        return ThrowException(String::New("Not enough arguments."));
    }
    else {
        Projection          *from   = ObjectWrap::Unwrap<Projection>(args[0]->ToObject());
        Projection          *to     = ObjectWrap::Unwrap<Projection>(args[1]->ToObject());
        Geometry            *geom   = ObjectWrap::Unwrap<Geometry>(args[2]->ToObject());
        PointTransformer    *trans  = new ProjectionPointTransformer(from->pj, to->pj);
        
        try {
            geom->ApplyPointTransformation(trans);
        }
        catch (TransformerException ex) {
            delete trans;
            return ThrowException(String::New(ex.GetDescription()));
        }
        
        delete trans;
    }
    
    return scope.Close(Null());
}

ProjectionPointTransformer::ProjectionPointTransformer(projPJ from, projPJ to) {
    this->from = pj_init_plus(pj_get_def(from, 0));
    this->to = pj_init_plus(pj_get_def(to, 0));
}

ProjectionPointTransformer::~ProjectionPointTransformer() {
    pj_free(this->from);
    pj_free(this->to);
}

void ProjectionPointTransformer::Transform(double *x, double *y, double *z) {
    if (pj_is_latlong(this->from)) {
        *x *= DEG_TO_RAD;
        *y *= DEG_TO_RAD;   
    }
    
    int err = pj_transform(this->from, this->to, 1, 1, x, y, z);
    
    if (pj_is_latlong(this->to)) {
        *x *= RAD_TO_DEG;
        *y *= RAD_TO_DEG;
    }
    
    if (err != 0) {
        throw TransformerException(pj_strerrno(err));
    }
}

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    initGEOS(notice_handler, error_handler);
    Geometry::Initialize(target);
    Projection::Initialize(target);
}

// TODO: where to call finishGEOS(); ? Is it even necessary?
