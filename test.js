var assert = require('assert');

var geonode = require("./geonode");
var sys = require("sys");

var rss = process.memoryUsage().rss;

function assertGeomsEquals(ga, gb) {
    return assert.ok(ga.equals(gb));
}

function assertGeomsEqualsExact(ga, gb, tol) {
    return assert.ok(ga.equalsExact(gb, tol));
}

var Geometry = geonode.Geometry;

var geom = new Geometry();

assert.ok(geom instanceof Geometry);

assert.ok(/^3\.[0-9.]+-CAPI-1\.[56]\.[0-9]$/.test(geom._geosVersion));

assert.equal(geom.toWkt(), "");

geom.fromWkt("POINT(0 0)");

// FIXME this kind of string matching can be fragile with WKTs
assert.equal(geom.toWkt(), "POINT (0.0000000000000000 0.0000000000000000)");

// You can also initialize a Geometry with a WKT passed to the constructor
var pt = new Geometry("POINT(1 1)");

assert.ok(pt instanceof Geometry);

assert.equal(pt.toWkt(), "POINT (1.0000000000000000 1.0000000000000000)");

var poly = new Geometry("POLYGON((0 0, 0 2, 2 2, 2 0, 0 0))");

polyWkt = "POLYGON ((0.0000000000000000 0.0000000000000000, 0.0000000000000000 2.0000000000000000, 2.0000000000000000 2.0000000000000000, 2.0000000000000000 0.0000000000000000, 0.0000000000000000 0.0000000000000000))";

assert.equal(poly.toWkt(), polyWkt);

assert.throws("new Geometry(\"SOMEGROSSLYINVALIDWKT\")");
assert.throws("var g = new Geometry(); g.fromWkt(\"SOMEGROSSLYINVALIDWKT\")");

assert.ok(poly.preparedContains(pt));
assert.ok(!pt.preparedContains(poly));
assert.ok(!poly.preparedContains(new Geometry("POINT(3 3)")));

assert.ok(!poly.isEmpty());
assert.ok(new Geometry("POINT EMPTY").isEmpty());

assert.ok(poly.isValid());
assert.ok(!new Geometry("POLYGON((0 0, 2 2, 0 2, 2 0, 0 0))").isValid());

assert.ok(poly.isSimple());

assert.ok(poly.preparedIntersects(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")));
assert.ok(!poly.preparedIntersects(new Geometry("LINESTRING(3 3, 4 4)")));
assert.ok(poly.preparedIntersects(new Geometry("POINT(0 0)")));

assert.ok(poly.preparedContainsProperly(pt));
assert.ok(poly.preparedContains(new Geometry("LINESTRING(0 0, 0 2, 1 1)")));
assert.ok(!poly.preparedContainsProperly(new Geometry("LINESTRING(0 0, 0 2, 1 1)")));
assert.ok(poly.preparedCovers(new Geometry("POINT(0 0)")));

poly.srid = 4326;
assert.equal(poly.srid, 4326);

assert.equal(poly.type, "Polygon");
assert.equal(new Geometry("POINT(0 0)").type, "Point");

assert.equal(poly.area, 4);

assert.equal(new Geometry("LINESTRING(0 0, 1 1)").length, Math.sqrt(2));

assert.equal(new Geometry("POINT(0 0)").distance(new Geometry("POINT(1 1)")), Math.sqrt(2));

assertGeomsEquals(new Geometry("POLYGON((0 1, 1 0, 2 1, 1 2, 0 1))").envelope, new Geometry("POLYGON ((0 0, 2 0, 2 2, 0 2, 0 0))"));

assertGeomsEquals(poly.intersection(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")), new Geometry("POLYGON((1 2, 2 2, 2 1, 1 1, 1 2))"));

var myCircle = new Geometry("POLYGON ((2.0000000000000000 1.0000000000000000, 1.9807852804032304 0.8049096779838719, 1.9238795325112870 0.6173165676349106, 1.8314696123025456 0.4444297669803983, 1.7071067811865481 0.2928932188134531, 1.5555702330196031 0.1685303876974553, 1.3826834323650909 0.0761204674887137, 1.1950903220161297 0.0192147195967698, 1.0000000000000016 0.0000000000000000, 0.8049096779838736 0.0192147195967692, 0.6173165676349122 0.0761204674887125, 0.4444297669803995 0.1685303876974537, 0.2928932188134541 0.2928932188134509, 0.1685303876974562 0.4444297669803957, 0.0761204674887143 0.6173165676349077, 0.0192147195967701 0.8049096779838688, 0.0000000000000000 0.9999999999999968, 0.0192147195967689 1.1950903220161249, 0.0761204674887118 1.3826834323650863, 0.1685303876974525 1.5555702330195991, 0.2928932188134495 1.7071067811865446, 0.4444297669803942 1.8314696123025427, 0.6173165676349064 1.9238795325112852, 0.8049096779838678 1.9807852804032295, 0.9999999999999962 2.0000000000000000, 1.1950903220161249 1.9807852804032311, 1.3826834323650867 1.9238795325112881, 1.5555702330195995 1.8314696123025469, 1.7071067811865455 1.7071067811865497, 1.8314696123025438 1.5555702330196044, 1.9238795325112859 1.3826834323650921, 1.9807852804032300 1.1950903220161304, 2.0000000000000000 1.0000000000000000))");
assertGeomsEqualsExact(new Geometry("POINT(1 1)").buffer(1), myCircle, 0.01);

assertGeomsEquals(new Geometry("POINT(1 1)").buffer(1, 1), new Geometry("POLYGON ((2.0000000000000000 1.0000000000000000, 1.0000000000000016 0.0000000000000000, 0.0000000000000000 0.9999999999999968, 0.9999999999999953 2.0000000000000000, 2.0000000000000000 1.0000000000000000))"));

assertGeomsEquals(new Geometry("POLYGON((1 1, 3 2, 2 1, 3 0, 1 1))").convexHull, new Geometry("POLYGON ((3 0, 1 1, 3 2, 3 0))"));

assertGeomsEquals(poly.difference(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")), new Geometry("POLYGON ((0 0, 0 2, 1 2, 1 1, 2 1, 2 0, 0 0))"));

assertGeomsEquals(poly.symDifference(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")), new Geometry("MULTIPOLYGON (((0 0, 0 2, 1 2, 1 1, 2 1, 2 0, 0 0)), ((2 1, 2 2, 1 2, 1 3, 3 3, 3 1, 2 1)))"));

assertGeomsEquals(new Geometry("LINESTRING(1 1, 0 0, -1 1)").boundary, new Geometry("MULTIPOINT(1 1, -1 1)"));

assertGeomsEquals(poly.union(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")), new Geometry("POLYGON ((0 0, 0 2, 1 2, 1 3, 3 3, 3 1, 2 1, 2 0, 0 0))"));

assertGeomsEquals(poly.pointOnSurface, new Geometry("POINT(1 1)"));

assertGeomsEquals(poly.centroid, new Geometry("POINT(1 1)"));

assertGeomsEquals(myCircle.simplify(0.1), new Geometry("POLYGON ((2.0000000000000000 1.0000000000000000, 1.7071067811865481 0.2928932188134531, 1.0000000000000016 0.0000000000000000, 0.2928932188134541 0.2928932188134509, 0.0000000000000000 0.9999999999999968, 0.2928932188134495 1.7071067811865446, 0.9999999999999962 2.0000000000000000, 1.7071067811865455 1.7071067811865497, 2.0000000000000000 1.0000000000000000))"));

assert.equal(poly.relate(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")), "212101212");

assert.ok(poly.relate(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))"), "212101212"));

//////////////////////

var Projection = geonode.Projection;
var lonlat = new Projection("+init=epsg:4326");

// Sanity checks, we're getting good stuff
assert.ok(lonlat instanceof Projection);
assert.equal(lonlat.definition, " +init=epsg:4326 +proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs +towgs84=0,0,0");

assert.throws('new Projection();');
assert.throws('new Projection("INVALID_PROJECTION");');

assert.throws('Projection.transform();');

// We're going to use the "World Mercator" projection for testing
var worldmerc = new Projection("+proj=merc +lat_ts=0 +lon_0=0 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs +towgs84=0,0,0")

// Test Point Projection
Projection.transform(lonlat, worldmerc, pt);
assert.equal("POINT (111319.4907932735659415 110579.9652218968840316)", pt.toWkt());

// Test Line Projection
var line = new Geometry("LINESTRING(0 0, 1 1)");
Projection.transform(lonlat, worldmerc, line);
assert.equal("LINESTRING (0.0000000000000000 0.0000000007081155, 111319.4907932735659415 110579.9652218968840316)", line.toWkt());

// Test Polygon Projection
var polyProj = new Geometry("POLYGON((1 1,5 1,5 5,1 5,1 1),(2 2, 3 2, 3 3, 2 3,2 2))");
Projection.transform(lonlat, worldmerc, polyProj);
assert.equal("POLYGON ((111319.4907932735659415 110579.9652218968840316, 556597.4539663678733632 110579.9652218968840316, 556597.4539663678733632 553583.8468157640891150, 111319.4907932735659415 553583.8468157640891150, 111319.4907932735659415 110579.9652218968840316), (222638.9815865471318830 221194.0771677151496988, 333958.4723798207123764 221194.0771677151496988, 333958.4723798207123764 331876.5342131220386364, 222638.9815865471318830 331876.5342131220386364, 222638.9815865471318830 221194.0771677151496988))",
    polyProj.toWkt());

// Test Polygon Projection Backwards    
Projection.transform(worldmerc, lonlat, polyProj);
assertGeomsEqualsExact(new Geometry("POLYGON ((0.9999999999999998 0.9999999999865795, 5.0000000000000000 0.9999999999865795, 5.0000000000000000 4.9999999999995621, 0.9999999999999998 4.9999999999995621, 0.9999999999999998 0.9999999999865795), (1.9999999999999996 1.9999999999732607, 3.0000000000000000 1.9999999999732607, 3.0000000000000000 2.9999999999997500, 1.9999999999999996 2.9999999999997500, 1.9999999999999996 1.9999999999732607))"),
    polyProj, 0.001);
    
//////////////////////

sys.puts("Heap increased by " + ((process.memoryUsage().rss - rss) / 1024) + " KB");
sys.puts("Tests pass!");
