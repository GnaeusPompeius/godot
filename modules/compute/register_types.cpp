/* register_types.cpp */

#include "register_types.h"
#include "core/class_db.h"
#include "map_gen.h"

void register_compute_types() {
	ClassDB::register_class<MapGenerator>();
}

void unregister_compute_types() {
   //nothing to do here
}
