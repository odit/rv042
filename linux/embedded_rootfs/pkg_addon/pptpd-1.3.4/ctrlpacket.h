/*
 * ctrlpacket.h
 *
 * Functions to parse and send pptp control packets.
 *
 * $Id: ctrlpacket.h 1603 2008-05-19 11:41:40Z david $
 */

#ifndef _PPTPD_CTRLPACKET_H
#define _PPTPD_CTRLPACKET_H

int read_pptp_packet(int clientFd, unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
size_t send_pptp_packet(int clientFd, unsigned char *packet, size_t packet_size);
void make_echo_req_packet(unsigned char *rply_packet, ssize_t * rply_size, u_int32_t echo_id);
void make_call_admin_shutdown(unsigned char *rply_packet, ssize_t * rply_size);
void make_stop_ctrl_req(unsigned char *rply_packet, ssize_t * rply_size);

#endif	/* !_PPTPD_CTRLPACKET_H */
