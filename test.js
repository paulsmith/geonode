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

// FIXME this kind of string matching is fragile with WKTs
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

sys.puts("Tests pass!");
