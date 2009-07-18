/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 */

#ifndef	_PlayerAction_H_
#define	_PlayerAction_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum PlayerAction {
	PlayerAction_actionNone	= 0,
	PlayerAction_actionFold	= 1,
	PlayerAction_actionCheck	= 2,
	PlayerAction_actionCall	= 3,
	PlayerAction_actionBet	= 4,
	PlayerAction_actionRaise	= 5,
	PlayerAction_actionAllIn	= 6
} e_PlayerAction;

/* PlayerAction */
typedef long	 PlayerAction_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PlayerAction;
asn_struct_free_f PlayerAction_free;
asn_struct_print_f PlayerAction_print;
asn_constr_check_f PlayerAction_constraint;
ber_type_decoder_f PlayerAction_decode_ber;
der_type_encoder_f PlayerAction_encode_der;
xer_type_decoder_f PlayerAction_decode_xer;
xer_type_encoder_f PlayerAction_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _PlayerAction_H_ */