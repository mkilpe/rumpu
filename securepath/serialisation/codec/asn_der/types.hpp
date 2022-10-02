#ifndef SECUREPATH_SERIALISATION_CODEC_ASN_DER_TYPES_HEADER
#define SECUREPATH_SERIALISATION_CODEC_ASN_DER_TYPES_HEADER

#include <cstdint>

namespace securepath::serialisation {

namespace asn_class {

enum {
	universal_c = 0x0,
	application_c = 0x1,
	context_specific_c = 0x2,
	private_c = 0x3
};

}

using asn_class_type = std::uint_fast8_t;

namespace asn_tag {

enum {
	boolean = 0x01,
	integer = 0x02,
	octet_string = 0x04,
	real = 0x09,
	sequence = 0x10,
	t61string = 0x14,
	generalised_time = 0x18
};

}

struct asn_header {
	asn_class_type asn_class;
	std::uint64_t tag;
	std::uint64_t length;
	bool is_constructed = false;
};

}

#endif
