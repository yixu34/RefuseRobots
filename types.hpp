#ifndef TYPES_HPP
#define TYPES_HPP

// Simple typedefs and prototypes for classes.
// While these really belong in more specific headers, distributing them
// that way would create lots of inclusion dependencies, some circular.


// unitIDs are unique global identifiers for units, meaning
//   (1) they can never be reused. Once a unit has been created with an ID
//       there can never be another with the same ID in the same game.
//   (2) They can be passed freely across the network, and will always refer
//       to the same thing no matter which computer is asking.
//   (3) They might not be closely packed, so an array with the Nth spot
//       corresponding to unitID N is a bad idea.
// The unitID zero (0) is reserved to mean null (no unit, or unitID not yet
// assigned to this unit.) It is an error to pass a unitID of zero except
// in a few cases.
typedef unsigned int unitID;

typedef unsigned int playerID;


#endif
