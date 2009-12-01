process.mixin(require("mjsunit"));

var geonode = require("./geonode");
var sys = require("sys");

var Geometry = geonode.Geometry;

var geom = new Geometry();

assertInstanceof(geom, Geometry);

// FIXME this match should be more flexible -- other users probably
// won't have this exact version of the GEOS C library
assertEquals(geom.version, "3.1.1-CAPI-1.6.0"); 

assertEquals(geom.toWkt(), "");

geom.fromWkt("POINT(0 0)");

// FIXME this kind of string matching can be fragile with WKTs
assertEquals(geom.toWkt(), "POINT (0.0000000000000000 0.0000000000000000)");

// You can also initialize a Geometry with a WKT passed to the constructor
var pt = new Geometry("POINT(1 1)");

assertInstanceof(pt, Geometry);

assertEquals(pt.toWkt(), "POINT (1.0000000000000000 1.0000000000000000)");

var poly = new Geometry("POLYGON((0 0, 0 2, 2 2, 2 0, 0 0))");

polyWkt = "POLYGON ((0.0000000000000000 0.0000000000000000, 0.0000000000000000 2.0000000000000000, 2.0000000000000000 2.0000000000000000, 2.0000000000000000 0.0000000000000000, 0.0000000000000000 0.0000000000000000))";

assertEquals(poly.toWkt(), polyWkt);

assertThrows("new Geometry(\"SOMEGROSSLYINVALIDWKT\")");
assertThrows("var g = new Geometry(); g.fromWkt(\"SOMEGROSSLYINVALIDWKT\")");

assertTrue(poly.contains(pt));
assertFalse(pt.contains(poly));
assertFalse(poly.contains(new Geometry("POINT(3 3)")));

assertTrue(!poly.isEmpty());
assertTrue(new Geometry("POINT EMPTY").isEmpty());

assertTrue(poly.isValid());
assertFalse(new Geometry("POLYGON((0 0, 2 2, 0 2, 2 0, 0 0))").isValid());

assertTrue(poly.isSimple());

assertTrue(poly.intersects(new Geometry("POLYGON((1 1, 1 3, 3 3, 3 1, 1 1))")));
assertFalse(poly.intersects(new Geometry("LINESTRING(3 3, 4 4)")));
assertTrue(poly.intersects(new Geometry("POINT(0 0)")));

poly.srid = 4326;
assertEquals(poly.srid, 4326);

assertEquals(poly.type, "Polygon");
assertEquals(new Geometry("POINT(0 0)").type, "Point");

assertEquals(poly.area, 4);

assertEquals(new Geometry("LINESTRING(0 0, 1 1)").length, Math.sqrt(2));

assertEquals(new Geometry("POINT(0 0)").distance(new Geometry("POINT(1 1)")), Math.sqrt(2));

assertEquals(new Geometry("POLYGON((0 1, 1 0, 2 1, 1 2, 0 1))").envelope.toWkt(), "POLYGON ((0.0000000000000000 0.0000000000000000, 2.0000000000000000 0.0000000000000000, 2.0000000000000000 2.0000000000000000, 0.0000000000000000 2.0000000000000000, 0.0000000000000000 0.0000000000000000))");

sys.puts("Tests pass!");