// Copyright (C) 2002 Adam Megacz <adam@xwt.org> all rights reserved.
//
// You may modify, copy, and redistribute this code under the terms of
// the GNU Library Public License version 2.1, with the exception of
// the portion of clause 6a after the semicolon (aka the "obnoxious
// relink clause")

package org.xwt;

import org.bouncycastle.crypto.AsymmetricBlockCipher;
import org.bouncycastle.crypto.Digest;
import org.bouncycastle.crypto.CipherParameters;
import org.bouncycastle.crypto.InvalidCipherTextException;
import org.bouncycastle.crypto.params.RSAKeyParameters;
import org.bouncycastle.crypto.params.AsymmetricKeyParameter;
import org.bouncycastle.crypto.params.KeyParameter;
import org.bouncycastle.crypto.digests.SHA1Digest;
import org.bouncycastle.crypto.digests.MD5Digest;
import org.bouncycastle.crypto.digests.MD2Digest;
import org.bouncycastle.crypto.engines.RSAEngine;
import org.bouncycastle.crypto.engines.RC4Engine;
import org.bouncycastle.util.encoders.Base64;
import org.bouncycastle.asn1.DERInputStream;
import org.bouncycastle.asn1.DEROutputStream;
import org.bouncycastle.asn1.DERConstructedSequence;
import org.bouncycastle.asn1.DERObject;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.BERInputStream;
import org.bouncycastle.asn1.x509.X509CertificateStructure;
import org.bouncycastle.asn1.x509.RSAPublicKeyStructure;
import org.bouncycastle.asn1.x509.SubjectPublicKeyInfo;
import org.bouncycastle.asn1.x509.TBSCertificateStructure;
import org.bouncycastle.asn1.x509.X509Name;
import org.bouncycastle.asn1.x509.X509Extensions;
import org.bouncycastle.asn1.x509.X509Extension;
import org.bouncycastle.asn1.x509.BasicConstraints;
import java.net.*;
import java.io.*;
import java.util.*;
import java.math.*;
import java.text.*;

/**

   TinySSL: a tiny SSL implementation in Java, built on the
            bouncycastle.org lightweight crypto library.

   This class implements an SSLv3 client-side socket, with the
   SSL_RSA_EXPORT_WITH_RC4_40_MD5 and SSL_RSA_WITH_RC4_128_MD5 cipher
   suites, as well as certificate chain verification against a
   collection of 93 built-in Trusted Root CA public keys (the same 93
   included with Microsoft Internet Explorer 5.5 SP2).

   As of 07-Dec-01, the zipped bytecode for this class is 43k, and the
   subset of bouncycastle it requires is 82k.

   This class should work correctly on any Java 1.1 compliant
   platform. The java.security.* classes are not used.

   The main design goal for this class was the smallest possible body
   of code capable of connecting to 99% of all active HTTPS
   servers. Although this class is useful in many other situations
   (IMAPS, Secure SMTP, etc), the author will refuse all feature
   requests and submitted patches which go beyond this scope.

   Because of the limited goals of this class, certain abstractions
   have been avoided, and certain parameters have been
   hard-coded. "Magic numbers" are often used instead of "static final
   int"'s, although they are usually accompanied by a descriptive
   comment. Numeric offsets into byte arrays are also favored over
   DataInputStream(ByteArrayInputStream(foo))'s.

   Much thanks and credit go to the BouncyCastle team for producing
   such a first-class library, and for helping me out on the
   dev-crypto mailing list while I was writing this.

   Revision History:

   1.0  07-Dec-01  Initial Release

   1.01 15-Mar-02  Added PKCS1 class to avoid dependancy on java.security.SecureRandom

   1.02 27-Mar-02  Fixed a bug which would hang the connection when more than one
                   Handshake message appeared in the same TLS Record

   1.03 10-Aug-02  Fixed a vulnerability outlined at
                   http://online.securityfocus.com/archive/1/286290

*/

public class TinySSL extends Socket {

    // Simple Test //////////////////////////////////////////////

    public static void main(String[] args) {
        //Log.on = true;
        try {
            //Socket s = new TinySSL("www.paypal.com", 443);
            Socket s = new TinySSL("new1.com", 443, true, true);
            System.out.println("Connected to new.com");

            PrintWriter pw = new PrintWriter(s.getOutputStream());
            BufferedReader br = new BufferedReader(new InputStreamReader(s.getInputStream()));
            //pw.println("GET / HTTP/1.0");
            pw.println("CONNECT localhost:25 HTTP/1.0");
            pw.println("");
            pw.flush();
            
            while(true) {
                String s2 = br.readLine();
                if (s2 == null) return;
                System.out.println(s2);
            }
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Static Data //////////////////////////////////////////////

    public static class SSLException extends IOException { public SSLException(String s) { super(s); } }
    static SubjectPublicKeyInfo[] trusted_CA_public_keys;
    static String[] trusted_CA_public_key_identifiers;
    public static byte[] pad1 = new byte[48];
    public static byte[] pad2 = new byte[48];
    public static byte[] pad1_sha = new byte[40];
    public static byte[] pad2_sha = new byte[40];
    static byte[] randpool;
    static long randcnt = 0;

    // Cipher State //////////////////////////////////////////////

    public byte[] server_random = new byte[32];
    public byte[] client_random = new byte[32];
    public byte[] client_write_MAC_secret = new byte[16];        
    public byte[] server_write_MAC_secret = new byte[16];        
    public byte[] client_write_key = null;
    public byte[] server_write_key = null;
    public byte[] master_secret = null;

    /** the bytes of the ServerKeyExchangeMessage, null if none recieved */
    public byte[] serverKeyExchange = null;

    /** true iff the server asked for a certificate */
    public boolean cert_requested = false;

    public X509CertificateStructure server_cert = null;

    public SSLOutputStream os = null;
    public SSLInputStream is = null;

    String hostname;

    /** if true, we don't mind if the server's cert isn't signed by a CA. USE WITH CAUTION! */
    //boolean ignoreUntrustedCert = false;
    boolean ignoreUntrustedCert = false;

    /** the concatenation of all the bytes of all handshake messages sent or recieved */
    public byte[] handshakes = new byte[] { };

    /** true iff we're using SSL_RSA_EXPORT_WITH_RC4_40_MD5 */
    boolean export = false;

    public InputStream getInputStream() throws IOException { return is != null ? is : super.getInputStream(); }
    public OutputStream getOutputStream() throws IOException { return os != null ? os : super.getOutputStream(); }

    public TinySSL(String host, int port) throws IOException { this(host, port, true, false); }
    public TinySSL(String host, int port, boolean negotiateImmediately) throws IOException { this(host, port, negotiateImmediately, false); }
    public TinySSL(String host, int port, boolean negotiateImmediately, boolean ignoreUntrustedCert) throws IOException {
        super(host, port);
        hostname = host;
        this.ignoreUntrustedCert = ignoreUntrustedCert;
        if (negotiateImmediately) negotiate();
    }

    /** negotiates the SSL connection */
    public void negotiate() throws IOException {
        os = new SSLOutputStream(super.getOutputStream());
        is = new SSLInputStream(super.getInputStream());
        os.writeClientHello();
        is.readServerHandshakes();
        os.sendClientHandshakes();
        is.readServerFinished();
    }

    class SSLInputStream extends InputStream {
        
        /** the underlying inputstream */
        DataInputStream raw;

        /** the server's sequence number */
        public int seq_num = 0;

        /** the decryption engine */
        public RC4Engine rc4 = null;
        
        /** pending bytes -- decrypted, but not yet fed to consumer */
        byte[] pend = null;
        int pendstart = 0;
        int pendlen = 0;

        public void mark() { }
        public void reset() { }
        public boolean markSupported() { return false; }
        public long skip(long l) throws IOException { for(long i=0; i<l; i++) read(); return l; }
        public SSLInputStream(InputStream raw) { this.raw = new DataInputStream(raw); }
        public int available() throws IOException { return pendlen; }

        public int read() throws IOException {
            byte[] singlebyte = new byte[1];
            int numread = read(singlebyte);
            if (numread != 1) return -1;
            return (int)singlebyte[0];
        }
       
        public int read(byte[] b, int off, int len) throws IOException {
            if (pendlen == 0) {
                pend = readRecord();
                if (pend == null) return -1;
                pendstart = 0;
                pendlen = pend.length;
            }
            int ret = Math.min(len, pendlen);
            System.arraycopy(pend, pendstart, b, off, ret);
            pendlen -= ret;
            pendstart += ret;
            return ret;
        }

        /** reads and decrypts exactly one record; blocks if unavailable */        
        public byte[] readRecord() throws IOException {

            // we only catch EOFException here, because anywhere else
            // would be "unusual", and we *want* and EOFException in
            // those cases
            byte type;
            try { type = raw.readByte();
            } catch (EOFException e) {
                System.out.println("got EOFException reading packet type");
                return null;
            }

            byte ver_major = raw.readByte();
            byte ver_minor = raw.readByte();
            short len = raw.readShort();
            //if (Log.on) Log.log(this, "got record of type " + type + ", SSLv" + ver_major + "." + ver_minor + ", length=" + len);

            byte[] ret = new byte[len];
            raw.readFully(ret);
            
            // simply ignore ChangeCipherSpec messages -- we change as soon as we send ours
            if (type == 20) {
                //if (Log.on) Log.log(this, "got ChangeCipherSpec; ignoring");
                seq_num = 0;
                return readRecord();
            }

            byte[] decrypted_payload;

            // if crypto hasn't been enabled yet; skip crypt and hash
            if (rc4 == null) decrypted_payload = ret;
            else {
                // decrypt the payload
                decrypted_payload = new byte[len - 16];
                rc4.processBytes(ret, 0, len - 16, decrypted_payload, 0);
                
                // check the MAC
                byte[] MAC = new byte[16];
                rc4.processBytes(ret, len - 16, 16, MAC, 0);
                byte[] ourMAC = computeMAC(type, decrypted_payload, 0, decrypted_payload.length, server_write_MAC_secret, seq_num++);
                for(int i=0; i<MAC.length; i++)
                    if (MAC[i] != ourMAC[i])
                        throw new SSLException("MAC mismatch on byte " + i + ": got " + MAC[i] + ", expecting " + ourMAC[i]);
            }

            if (type == 21) {
                if (decrypted_payload[1] > 1) {
                    throw new SSLException("got SSL ALERT message, level=" + decrypted_payload[0] + " code=" + decrypted_payload[1]);
                } else if (decrypted_payload[1] == 0) {
                    //if (Log.on) Log.log(this, "server requested connection closure; returning null");
                    return null;
                } else {
                    //if (Log.on) Log.log(this, "got SSL ALERT message, level=" + decrypted_payload[0] + " code=" + decrypted_payload[1]);
                    return readRecord();
                }

            } else if (type == 22) {
                //if (Log.on) Log.log(this, "read a handshake");

            } else if (type != 23) {
                //if (Log.on) Log.log(this, "unexpected record type: " + type + "; skipping");
                return readRecord();

            }
                
            //if (Log.on) Log.log(this, "  returning " + decrypted_payload.length + " byte record payload");
            return decrypted_payload;
        }

        private byte[] readHandshake() throws IOException {
            // acquire a handshake message
            byte type = (byte)read();
            int len = ((read() & 0xff) << 16) | ((read() & 0xff) << 8) | (read() & 0xff);
            byte[] rec = new byte[len + 4];
            rec[0] = type;
            rec[1] = (byte)(((len & 0x00ff0000) >> 16) & 0xff);
            rec[2] = (byte)(((len & 0x0000ff00) >> 8) & 0xff);
            rec[3] = (byte)((len & 0x000000ff) & 0xff);
            if (len > 0) read(rec, 4, len);
            return rec;
        }

        /** This reads the ServerHello, Certificate, and ServerHelloDone handshake messages */
        public void readServerHandshakes() throws IOException {
            for(;;) {

                byte[] rec = readHandshake();
                handshakes = concat(new byte[][] { handshakes, rec });
                DataInputStream stream = new DataInputStream(new ByteArrayInputStream(rec, 4, rec.length - 4));

                switch(rec[0]) {
                case 2: // ServerHello
                    //if (Log.on) Log.log(this, "got ServerHello");
                    byte ver_major = rec[4];
                    byte ver_minor = rec[5];
                    System.arraycopy(rec, 6, server_random, 0, server_random.length);
                    short cipher_high = rec[6 + server_random.length + rec[6 + server_random.length] + 1];
                    short cipher_low = rec[6 + server_random.length + rec[6 + server_random.length] + 2];

                    if (cipher_low == 0x04 || cipher_high != 0x00) {
                        export = false;
                        //if (Log.on) Log.log(this, "using SSL_RSA_WITH_RC4_128_MD5");

                    } else if (cipher_low == 0x03 || cipher_high != 0x00) {
                        export = true;
                        //if (Log.on) Log.log(this, "using SSL_RSA_EXPORT_WITH_RC4_40_MD5");

                    } else throw new SSLException("server asked for cipher " + ((cipher_high << 8) | cipher_low) +
                                                " but we only do SSL_RSA_WITH_RC4_128_MD5 (0x0004) and " +
                                                "SSL_RSA_EXPORT_WITH_RC4_40_MD5 (0x0003)");

                    byte compressionMethod = rec[6 + server_random.length + rec[6 + server_random.length] + 3];
                    if (compressionMethod != 0x0) throw new SSLException("server asked for compression method " + compressionMethod +
                                                                         " but we don't support compression");
                    break;
                    
                case 11: // Server's certificate(s)
                    //if (Log.on) Log.log(this, "got Server Certificate(s)");
                    int numcertbytes = ((rec[4] & 0xff) << 16) | ((rec[5] & 0xff) << 8) | (rec[6] & 0xff);
                    int numcerts = 0;
                    X509CertificateStructure last_cert = null;
                    X509CertificateStructure this_cert = null;

                    for(int i=0; i<numcertbytes;) {
                        int certlen = ((rec[7 + i] & 0xff) << 16) | ((rec[7 + i + 1] & 0xff) << 8) | (rec[7 + i + 2] & 0xff);
                        try {
                            DERInputStream dIn = new DERInputStream(new ByteArrayInputStream(rec, 7 + i + 3, certlen));
                            this_cert = new X509CertificateStructure((DERConstructedSequence)dIn.readObject());
                        } catch (Exception e) {
                            SSLException t = new SSLException("error decoding server certificate: " + e);
                            t.fillInStackTrace();
                            throw t;
                        }

                        if (server_cert == null) {
                            server_cert = this_cert;
                            TBSCertificateStructure tbs = server_cert.getTBSCertificate();
                            X509Name subject = tbs.getSubject();

                            // gross hack to extract the Common Name so we can compare it to the server hostname
                            String CN = tbs.getSubject().toString() + " ";
                            boolean good = false;
                            for(int j=0; j<CN.length() - 3; j++)
                                if (CN.substring(j, j+3).equals("CN=")) {
                                    good = true;
                                    CN = CN.substring(j+3, CN.indexOf(' ', j+3));
                                    break;
                                }

                            //if (!good) throw new SSLException("server certificate does not seem to have a CN: " + CN);
                            if (!good) System.out.println("server certificate does not seem to have a CN: " + CN);
                            if (!ignoreUntrustedCert && !CN.equals(hostname))
                                //throw new SSLException("connecting to host " + hostname + " but server certificate was issued for " + CN);
                                System.out.println("connecting to host " + hostname + " but server certificate was issued for " + CN);

                            SimpleDateFormat dateF = new SimpleDateFormat("MM-dd-yy-HH-mm-ss-z");

                            // the following idiocy is a result of the brokenness of the GNU Classpath's SimpleDateFormat
                            String s = tbs.getStartDate().getTime();
                            s = s.substring(2, 4) + "-" + s.substring(4, 6) + "-" + s.substring(0, 2) + "-" + s.substring(6, 8) + "-" +
                                s.substring(8, 10) + "-" + s.substring(10, 12) + "-" + s.substring(12);
                            Date startDate = dateF.parse(s, new ParsePosition(0));

                            s = tbs.getEndDate().getTime();
                            s = s.substring(2, 4) + "-" + s.substring(4, 6) + "-" + s.substring(0, 2) + "-" + s.substring(6, 8) + "-" +
                                s.substring(8, 10) + "-" + s.substring(10, 12) + "-" + s.substring(12);
                            Date endDate = dateF.parse(s, new ParsePosition(0));
/* XXX
                            Date now = new Date();
                            if (!ignoreUntrustedCert && now.after(endDate))
                                throw new SSLException("server certificate expired on " + endDate);
                            if (!ignoreUntrustedCert && now.before(startDate))
                                throw new SSLException("server certificate will not be valid until " + startDate);
*/

                            //Log.log(this, "server cert (name, validity dates) checks out okay");
                            
                        } else {

                            // don't check the top cert since some very old root certs lack a BasicConstraints field.
                            if (certlen + 3 + i < numcertbytes) {
                                // defend against Mike Benham's attack
                                X509Extension basicConstraints = this_cert.getTBSCertificate().getExtensions().getExtension(X509Extensions.BasicConstraints);
                                if (basicConstraints == null) throw new SSLException("certificate did not contain a basic constraints block");
                                DERInputStream dis = new DERInputStream(new ByteArrayInputStream(basicConstraints.getValue().getOctets()));
                                BasicConstraints bc = new BasicConstraints((DERConstructedSequence)dis.readObject());
                                if (!bc.isCA()) throw new SSLException("non-CA certificate used for signing");
                            }

                            if (!isSignedBy(last_cert, this_cert.getSubjectPublicKeyInfo()))
                                throw new SSLException("the server sent a broken chain of certificates");
                        }

                        last_cert = this_cert;
                        i += certlen + 3;
                        numcerts++;
                    }
                    //if (Log.on) Log.log(this, "  Certificate (" + numcerts + " certificates)");

                    if (ignoreUntrustedCert) break;

                    boolean good = false;

                    // pass 1 -- only check CA's whose subject is a partial match
                    String subject = this_cert.getSubject().toString();
                    for(int i=0; i<trusted_CA_public_keys.length; i++) {
                        if (subject.indexOf(trusted_CA_public_key_identifiers[i]) != -1 && isSignedBy(this_cert, trusted_CA_public_keys[i])) {
                            //if (Log.on) Log.log(this, "pass 1: server cert was signed by trusted CA " + i);
                            good = true;
                            break;
                        }
                    }

                    // pass 2 -- try all certs
                    if (!good)
                        for(int i=0; i<trusted_CA_public_keys.length; i++) {
                            if (isSignedBy(this_cert, trusted_CA_public_keys[i])) {
                                //if (Log.on) Log.log(this, "pass 2: server cert was signed by trusted CA " + i);
                                good = true;
                                break;
                            }
                        }

                    if (!good) throw new SSLException("server cert was not signed by a trusted CA");
                    break;

                case 12: 
                    //if (Log.on) Log.log(this, "got ServerKeyExchange");
                    serverKeyExchange = rec;
                    break;

                case 13:
                    //if (Log.on) Log.log(this, "got Request for Client Certificates");
                    cert_requested = true;
                    break;
                    
                case 14: //if (Log.on) Log.log(this, "  ServerHelloDone"); 
                                return;
                default: throw new SSLException("unknown handshake of type " + rec[0]);
                }
            }
        }
     
        public void readServerFinished() throws IOException {
            
            byte[] rec = readHandshake();
            if (rec[0] != 20) throw new SSLException("expecting server Finished message, but got message of type " + rec[0]);

            byte[] expectedFinished = concat(new byte[][] {
                md5(new byte[][] { master_secret, pad2,
                                   md5(new byte[][] { handshakes, new byte[] { (byte)0x53, (byte)0x52, (byte)0x56, (byte)0x52 },
                                                      master_secret, pad1 }) }),
                sha(new byte[][] { master_secret, pad2_sha,
                                   sha(new byte[][] { handshakes, new byte[] { (byte)0x53, (byte)0x52, (byte)0x56, (byte)0x52 },
                                                      master_secret, pad1_sha } ) } ) } );

            for(int i=0; i<expectedFinished.length; i++)
                if (expectedFinished[i] != rec[i + 4])
                    throw new SSLException("server Finished message mismatch!");

            //if (Log.on) Log.log(this, "server finished message checked out okay!");
        }
   
    }
    
    class SSLOutputStream extends OutputStream {
        
        /** the underlying outputstream */
        DataOutputStream raw;
        
        /** the sequence number for sending */
        public long seq_num = 0;

        /** the encryption engine for sending */
        RC4Engine rc4 = null;
        
        public SSLOutputStream(OutputStream raw) { this.raw = new DataOutputStream(raw); }
        public void flush() throws IOException { raw.flush(); }
        public void write(int b) throws IOException { write(new byte[] { (byte)b }, 0, 1); }
        public void write(byte[] b, int off, int len) throws IOException { write(b, off, len, (byte)23); }
        public void close() throws IOException {
            write(new byte[] { 0x1, 0x0 }, 0, 2, (byte)21);
            raw.close();
        }
        
        /** writes a single SSL Record */
        public void write(byte[] payload, int off, int len, byte type) throws IOException {

            // largest permissible frame is 2^14 octets
            if (len > 1 << 14) {
                write(payload, off, 1 << 14, type);
                write(payload, off + 1 << 14, len - 1 << 14, type);
                return;
            }

            raw.writeByte(type);
            raw.writeShort(0x0300);

            if (rc4 == null) {
                raw.writeShort(len);
                raw.write(payload, off, len);

            } else {
                byte[] MAC = computeMAC(type, payload, off, len, client_write_MAC_secret, seq_num);
                byte[] encryptedPayload = new byte[MAC.length + len];
                rc4.processBytes(payload, off, len, encryptedPayload, 0);
                rc4.processBytes(MAC, 0, MAC.length, encryptedPayload, len);
                raw.writeShort(encryptedPayload.length);
                raw.write(encryptedPayload);

            }

            seq_num++;
        }

        /** tacks a handshake header onto payload before sending it */        
        public void writeHandshake(int type, byte[] payload) throws IOException {
            byte[] real_payload = new byte[payload.length + 4];
            System.arraycopy(payload, 0, real_payload, 4, payload.length);
            real_payload[0] = (byte)(type & 0xFF);
            intToBytes(payload.length, real_payload, 1, 3);
            handshakes = concat(new byte[][] { handshakes, real_payload });
            write(real_payload, 0, real_payload.length, (byte)22);
        }

        public void sendClientHandshakes() throws IOException {
            
            //if (Log.on) Log.log(this, "shaking hands");
            if (cert_requested) {
                //if (Log.on) Log.log(this, "telling the server we have no certificates");
                writeHandshake(11, new byte[] { 0x0, 0x0, 0x0 });
            }
            
            // generate the premaster secret
            byte[] pre_master_secret = new byte[48];
            pre_master_secret[0] = 0x03;                            // first two bytes of premaster secret are our version number
            pre_master_secret[1] = 0x00;
            getRandomBytes(pre_master_secret, 2, pre_master_secret.length - 2);

            // encrypt and send the pre_master_secret            
            try {
                byte[] encrypted_pre_master_secret;

                SubjectPublicKeyInfo pki = server_cert.getSubjectPublicKeyInfo();
                RSAPublicKeyStructure rsa_pks = new RSAPublicKeyStructure((DERConstructedSequence)pki.getPublicKey());
                BigInteger modulus = rsa_pks.getModulus();
                BigInteger exponent = rsa_pks.getPublicExponent();

                if (serverKeyExchange != null) {

                    AsymmetricBlockCipher rsa = new PKCS1(new RSAEngine());
                    rsa.init(false, new RSAKeyParameters(false, modulus, exponent));

                    int modulus_size = ((serverKeyExchange[4] & 0xff) << 8) | (serverKeyExchange[5] & 0xff);
                    byte[] b_modulus = new byte[modulus_size];
                    System.arraycopy(serverKeyExchange, 6, b_modulus, 0, modulus_size);
                    modulus = new BigInteger(1, b_modulus);

                    int exponent_size = ((serverKeyExchange[6 + modulus_size] & 0xff) << 8) | (serverKeyExchange[7 + modulus_size] & 0xff);
                    byte[] b_exponent = new byte[exponent_size];
                    System.arraycopy(serverKeyExchange, 8 + modulus_size, b_exponent, 0, exponent_size);
                    exponent = new BigInteger(1, b_exponent);

                    byte[] server_params = new byte[modulus_size + exponent_size + 4];
                    System.arraycopy(serverKeyExchange, 4, server_params, 0, server_params.length);

                    byte[] expectedSignature = concat(new byte[][] { md5(new byte[][] { client_random, server_random, server_params } ),
                                                                     sha(new byte[][] { client_random, server_random, server_params } ) } );

                    byte[] recievedSignature = rsa.processBlock(serverKeyExchange, 6 + server_params.length,
                                                                serverKeyExchange.length - 6 - server_params.length);

                    for(int i=0; i<expectedSignature.length; i++)
                        if (expectedSignature[i] != recievedSignature[i])
                            throw new SSLException("ServerKeyExchange message had invalid signature " + i);

                    //if (Log.on) Log.log(this, "ServerKeyExchange successfully processed");
                }

                AsymmetricBlockCipher rsa = new PKCS1(new RSAEngine());
                rsa.init(true, new RSAKeyParameters(false, modulus, exponent));

                encrypted_pre_master_secret = rsa.processBlock(pre_master_secret, 0, pre_master_secret.length);
                writeHandshake(16, encrypted_pre_master_secret);

            } catch (Exception e) {
                SSLException t = new SSLException("exception encrypting premaster secret");
                t.fillInStackTrace();
                throw t;
            }
            
            // ChangeCipherSpec
            //if (Log.on) Log.log(this, "Handshake complete; sending ChangeCipherSpec");
            write(new byte[] { 0x01 }, 0, 1, (byte)20);
            seq_num = 0;

            // compute master_secret
            master_secret = concat(new byte[][] {
                md5(new byte[][] { pre_master_secret,
                                   sha(new byte[][] { new byte[] { 0x41 }, pre_master_secret, client_random, server_random })}),
                md5(new byte[][] { pre_master_secret,
                                   sha(new byte[][] { new byte[] { 0x42, 0x42 }, pre_master_secret, client_random, server_random })}),
                md5(new byte[][] { pre_master_secret,
                                   sha(new byte[][] { new byte[] { 0x43, 0x43, 0x43 }, pre_master_secret, client_random, server_random })})
                } );
            
            // construct the key material
            byte[] key_material = new byte[] { };
            for(int i=0; key_material.length < 72; i++) {
                byte[] crap = new byte[i + 1];
                for(int j=0; j<crap.length; j++) crap[j] = (byte)(((byte)0x41) + ((byte)i));
                key_material = concat(new byte[][] { key_material,
                                                   md5(new byte[][] { master_secret,
                                                                      sha(new byte[][] { crap, master_secret, server_random, client_random }) }) });
            }

            client_write_key = new byte[export ? 5 : 16];
            server_write_key = new byte[export ? 5 : 16];

            System.arraycopy(key_material, 0,  client_write_MAC_secret, 0, 16);
            System.arraycopy(key_material, 16, server_write_MAC_secret, 0, 16);
            System.arraycopy(key_material, 32, client_write_key, 0, export ? 5 : 16);
            System.arraycopy(key_material, export ? 37 : 48, server_write_key, 0, export ? 5 : 16);
            
            if (export) {
                // see SSLv3 spec, 6.2.2 for explanation
                byte[] client_untrimmed = md5(new byte[][] { concat(new byte[][] { client_write_key, client_random, server_random } ) });
                byte[] server_untrimmed = md5(new byte[][] { concat(new byte[][] { server_write_key, server_random, client_random } ) });
                client_write_key = new byte[16];
                server_write_key = new byte[16];
                System.arraycopy(client_untrimmed, 0, client_write_key, 0, 16);
                System.arraycopy(server_untrimmed, 0, server_write_key, 0, 16);
            }

            rc4 = new RC4Engine();
            rc4.init(true, new KeyParameter(client_write_key));
            is.rc4 = new RC4Engine();
            is.rc4.init(false, new KeyParameter(server_write_key));
            
            // send Finished
            writeHandshake(20, concat(new byte[][] { 
                md5(new byte[][] { master_secret, pad2, 
                                   md5(new byte[][] { handshakes, new byte[] { (byte)0x43, (byte)0x4C, (byte)0x4E, (byte)0x54 },
                                                      master_secret, pad1 }) }),
                sha(new byte[][] { master_secret, pad2_sha,
                                   sha(new byte[][] { handshakes, new byte[] { (byte)0x43, (byte)0x4C, (byte)0x4E, (byte)0x54 },
                                                      master_secret, pad1_sha } ) })
            }));
            raw.flush();
            //if (Log.on) Log.log(this, "wrote Finished message");

        }
        
        public void writeClientHello() throws IOException {
            
            //if (Log.on) Log.log(this, "sending ClientHello");
            int unixtime = (int)(System.currentTimeMillis() / (long)1000);
            
            byte[] out = new byte[] {
                0x03, 0x00,                     // client version (SSLv3.0)
                
                // space for random bytes
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0,
                
                0x0,                            // empty vector for sessionid
                0x0, 0x4, 0x0, 0x4, 0x0, 0x3,   // we support two ciphersuites: SSL_RSA_WITH_RC4_128_MD5 and SSL_RSA_EXPORT_WITH_RC4_40_MD5
                0x1, 0x0                        // we only support one compression method: none
            };
            
            // don't need to use secure random here since it's sent in the clear
            Random rand = new Random(System.currentTimeMillis());
            rand.nextBytes(client_random);
            intToBytes(unixtime, client_random, 0, 4);
            System.arraycopy(client_random, 0, out, 2, client_random.length);
            
            writeHandshake(1, out);
            flush();
        }
    }

    // Static Helpers ////////////////////////////////////////////////////////////////////

    /** copy the least significant num bytes of val into byte array b, startint at offset */
    public static void intToBytes(long val, byte[] b, int offset, int num) {
        for(int i=0; i<num; i++)
            b[offset + num - i - 1] = (byte)((val & (0xFFL << (i * 8))) >> (i * 8));
    }

    /** fills b with random bytes */
    public static synchronized void getRandomBytes(byte[] b, int offset, int len) {
        MD5Digest md5 = new MD5Digest();
        byte[] b2 = new byte[16];
        while(len > 0) {
            md5.reset();
            md5.update(randpool, 0, randpool.length);
            intToBytes(randcnt++, b2, 0, 8);
            md5.update(b2, 0, 8);
            md5.doFinal(b2, 0);
            int n = len < 16 ? len : 16;
            System.arraycopy(b2, 0, b, offset, n);
            len -= n;
            offset += n;
        }
    }

    public static byte[] computeMAC(byte type, byte[] payload, int off, int len, byte[] MAC_secret, long seq_num) {
        byte[] MAC = new byte[16];
        MD5Digest md5 = new MD5Digest();
        md5.update(MAC_secret, 0, MAC_secret.length);
        md5.update(pad1, 0, pad1.length);

        byte[] b = new byte[11];
        intToBytes(seq_num, b, 0, 8);
        b[8] = type;
        intToBytes(len, b, 9, 2);
        md5.update(b, 0, b.length);

        md5.update(payload, off, len);
        md5.doFinal(MAC, 0);
        md5.reset();
        md5.update(MAC_secret, 0, MAC_secret.length);
        md5.update(pad2, 0, pad2.length);
        md5.update(MAC, 0, MAC.length);
        md5.doFinal(MAC, 0);

        return MAC;
    }

    public static byte[] concat(byte[][] inputs) {
        int total = 0;
        for(int i=0; i<inputs.length; i++) total += inputs[i].length;
        byte[] ret = new byte[total];
        int pos = 0;
        for(int i=0; i<inputs.length; i++) {
            System.arraycopy(inputs[i], 0, ret, pos, inputs[i].length);
            pos += inputs[i].length;
        }
        return ret;
    }
    
    SHA1Digest master_sha1 = new SHA1Digest();
    public byte[] sha(byte[][] inputs) {
        master_sha1.reset();
        for(int i=0; i<inputs.length; i++) master_sha1.update(inputs[i], 0, inputs[i].length);
        byte[] ret = new byte[master_sha1.getDigestSize()];
        master_sha1.doFinal(ret, 0);
        return ret;
    }
    
    MD5Digest master_md5 = new MD5Digest();
    public byte[] md5(byte[][] inputs) {
        master_md5.reset();
        for(int i=0; i<inputs.length; i++) master_md5.update(inputs[i], 0, inputs[i].length);
        byte[] ret = new byte[master_md5.getDigestSize()];
        master_md5.doFinal(ret, 0);
        return ret;
    }

    // FEATURE: improve error reporting in here
    /** returns true iff certificate "signee" is signed by public key "signer" */
    public static boolean isSignedBy(X509CertificateStructure signee, SubjectPublicKeyInfo signer) throws SSLException {

        Digest hash = null;

        String signature_algorithm_oid = signee.getSignatureAlgorithm().getObjectId().getId();
        if (signature_algorithm_oid.equals("1.2.840.113549.1.1.4")) hash = new MD5Digest();
        else if (signature_algorithm_oid.equals("1.2.840.113549.1.1.2")) hash = new MD2Digest();
        else if (signature_algorithm_oid.equals("1.2.840.113549.1.1.5")) hash = new SHA1Digest();
        else throw new SSLException("unsupported signing algorithm: " + signature_algorithm_oid);

        try {
            // decrypt the signature using the signer's public key
            byte[] ED = signee.getSignature().getBytes();
            SubjectPublicKeyInfo pki = signer;
            RSAPublicKeyStructure rsa_pks = new RSAPublicKeyStructure((DERConstructedSequence)pki.getPublicKey());
            BigInteger modulus = rsa_pks.getModulus();
            BigInteger exponent = rsa_pks.getPublicExponent();
            AsymmetricBlockCipher rsa = new PKCS1(new RSAEngine());
            rsa.init(false, new RSAKeyParameters(false, modulus, exponent));
            
            // Decode the embedded octet string
            byte[] D = rsa.processBlock(ED, 0, ED.length);
            BERInputStream beris = new BERInputStream(new ByteArrayInputStream(D));
            DERObject derob = beris.readObject();
            DERConstructedSequence dercs = (DERConstructedSequence)derob;
            DEROctetString deros = (DEROctetString)dercs.getObjectAt(1);
            byte[] MD = deros.getOctets();
            
            // generate our own hash
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            DEROutputStream dos = new DEROutputStream(baos);
            dos.writeObject(signee.getTBSCertificate());
            dos.flush();
            byte[] b = baos.toByteArray();
            hash.update(b, 0, b.length);
            byte[] md_out = new byte[MD.length];
            hash.doFinal(md_out, 0);
            
            // compare our hash to the signed hash
            for(int j=0; j<MD.length; j++) if (md_out[j] != MD[j]) return false;
            return true;

        } catch (Exception e) {
            return false;

        }
    }

    // Embedded Trusted Public Keys //////////////////////////////////////////////

    /** base64-encoded sequence of DER-encoded PKCS7 certs for all the "trusted root CA's" included with IE5.5 */
    static String[] base64_encoded_trusted_CA_public_keys = new String[] {

        "CN=ABA.ECOM Root CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsdMR4HlVQwcITMsFQgDiDYNGPe" +
        "STurYG0w1ZvT7BzkNnAYohqO+8zNCizLBVllOEZgUA2kRJgNhUCqUlhpTtY1b/cGyjoRnS" +
        "eL5oKkReL8/MGF5HvDqxRj0e8LksNF+MfEwIKZ1AVes8fYPetfD3ioMOoUy0OqWzX1oil+" +
        "wZm8EFaP3mt6mRlCzkeEgkGiUZOuuVnDkKis9CsvAc1V/7a+1oVns5LHI4sO6TqdN7dzzr" +
        "cQOpOEoWbIkqytozE3nCVYztnLvyy1sQ+C5hNcYpTCrQKmPRZVm0+M359ACEtldChZ0yqP" +
        "kqVPv/eEG8vXEo9LuQvP+WNATjRZ6hRihAgQIDAQAB",

        "O=ViaCode",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQCws2enlrV2g+kWA9kaoHbVdOecBdJRP9" +
        "tPGaGoOU7LJH1hxB5qIK4Pgd7quDn9Gx9rNkDtTSEl8qPZoVHYbMAblvjUQpTUp84bj9NU" +
        "JqKE7zKFr0o/8TI2rz3mOifrA8IlfvRhK62KGkvmmzZo1C/l0oiU3Baq2sIVTGzD4RmRyQ" +
        "IBAw==",

        "CN=Xcert EZ by DST",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArVQY3rS/963odKrti3yPwtR1Gt" +
        "WEubZi/Inv5JdhkvsduOFaRzSengYi+9PqOMu4iwf3GqAXdwdaMBzUKTgg1ydA2FCTQ7/S" +
        "GKIpdgVyqmu2aZireR4cZfVqi/zFFqqictpg7U5uGSV6Ch0w41CbQjxE66GwIB7bAn7+PR" +
        "+/0ACK20B2philFadXtlLCAReYd4+KgcYatGoq5q+p1gCsz9gVSXzbG6H+gfqH+dOQwQLA" +
        "+dBC6ZFoJV/Gv4c56ZUAYCi/gyzA51621zYW52CHdujnJ7IlDYt65aod5VnNzgsOb8bInO" +
        "MQ2YU507eb+sa6fHTSXXVWq3SkolG/UnzucQIDAQAB",

        "CN=Certiposte Classe A Personne",
        "MIIBITANBgkqhkiG9w0BAQEFAAOCAQ4AMIIBCQKCAQAox3xaJSN48rMAR0Biy2+MQlCfnl" +
        "7UXA5lC1hWlSvjRtBhNuAtRpuCy5Hu0pV8mpKvBAp+pp/g17HDRfmYQRs5redW19m2f867" +
        "OS4sO8+2cwODzhNdMmpjottb+Esz6FBsy6gX7J6TuWwGSyYLdx6e+eWMiTfS0bv9qYwrLJ" +
        "wQMdhLjM23cX44LCnjF7JP6FK245I80v3hAtphEHTSGvPI0dFmB1/EhGNpva5s3GUjHLf7" +
        "98YTLoN+P6nlCyBtAQo34lzait4icOkN4HQ9xOtxm2Eq4g0Ui0xGN0wm0mjWVsNXqqJgN6" +
        "9fnaCzgILmQypMgAAJUNmoanNtA/5ec5LlAgMBAAE=",

        "CN=Certiposte Serveur",
        "MIIBITANBgkqhkiG9w0BAQEFAAOCAQ4AMIIBCQKCAQA+p3gzOJHiylaV0ZFGsiPcpVZ/D8" +
        "eXuOKekS4oFi6O80e2XIPE8Ob+ZxqTZH1ACdgdaADs1BHu2GOJAyPphF/HVQ5K4nK7KcFV" +
        "ZHao45LN9/ZuQlYYUjOJ+YAUqBlRfsd3v3qoMcB9F25DTtVmyQU+S+Ll4lUbdKpRHarMmB" +
        "F3pOvbKg4nx9XNSOzcfk5J50HNmQvRS14YGw06CpstmznHQAzQdgd8fI9+XHKOh9W+8qa5" +
        "3r/dnxJ5R3zFyZdARgCS0xNak0+dfthfTMFdSEnZLZg8/MynhyHwPo5yfVk4NhYaDEi+of" +
        "LVPqgWDCBZz84PM4M9rav1/93X/WkIiADvAgMBAAE=",

        "OU=Certisign - Autoridade Certificadora - AC2",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC5MMyl65DWVpRnM4mDbUa+cJeTF04KJ3" +
        "DOycXyxdIt0RGcdzJsdNOSb/rp1bhhmqpMEz41OvDuCTbZ0Zcxx16sQUm/SG1OIFPJe2qj" +
        "ljFrsm6ozy9yTAatMs9aCPN9EJyqu7pz+fPwuCRvqGW2Iv4FWxBVRMIDHa3RIswIbfuMyw" +
        "IDAQAB",

        "OU=Certisign - Autoridade Certificadora - AC4",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDsg9TMg5A/X+y+wenQx1hGWR/xk0qyFx" +
        "MLzymZqwRFM+PRXr68jiV3Yt2bkpsxCkBFedXys91suUD9mH9Aoi3pspO9S9XB3unR+nH3" +
        "P0G89BSvzWvIOUqdYGW0hNBqQeljrptp6rlGHNsYCDtiTN5B156GfxNyEdTc6t5gpbvdGw" +
        "IDAQAB",

        "OU=Certisign Autoridade Certificadora AC1S",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCwwJXro8VB+JtvcWOOkRFX+QPHaJoanG" +
        "Hwww8Ml2KIfiYBNX398W9PF5WqfvK7vO/idnNhlTZRgz6E6D+6VzY3lBNskmQflA3rVC9R" +
        "WuUoXvCShufkbSF6XzcL51u9LQKogfk/yxTIvKTF49HLN9yr5Yeq8guYLnrPzB7Cf+j9AQ" +
        "IDAQAB",

        "OU=Certisign Autoridade Certificadora AC3S",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDOZE7Wz658mCeY7yjvujTDNRqd0mYecf" +
        "Hkli0nFzmQRY8t7+bVR6nhg4F8Pihx+oC7XfhDaxkQwZhvFZ4trklkROyEGmlZFleyPZLY" +
        "Zku/ma1DGMc4yYuOLAQus0trk/adH4SyzeYAwr42pbxZtZ+LGSD/5agopFW2irayxddE4w" +
        "IDAQAB",

        "O=Certplus, CN=Class 1 Primary CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw2spyC7HrnxSBemTiVYKWnnJzN" +
        "wl74eKLQXYgRcEGzpF+HkODUnUgUHIq0X7dcgV8uLQvNlhbISkExmn2fnySdxMD8Z9V7QT" +
        "3B4JcSk2nYBY9BvYiRTr09KTSyrxd+dqZb0Z5ar9DEpj4cKZtA8EtlobNjw3PL/F5V7xX1" +
        "cOH8f9LOfkb2qbYpY5EZtm8Cy2UtzhJ//bbf7rq2MUHWOIY+IWDPkgVA+b3RVqdoNPvSeL" +
        "U6Y30ofyR1BSO2bp0XgaG7I7afBZPDhb0SpMM14Oylal7S1bgoNN1jhOila2ai8kaxIwpi" +
        "rerwy7qkQSHBPFZQ/j/dgaMUvkPwx8RegWMwIDAQAB",

        "O=Certplus, CN=Class 2 Primary CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3FCW0BL4NdIIeHq2UnD9b+7PuR" +
        "HLXXfh7Ol+BI3WzG9zQ1dgrDMKROwDXxyAJJHlqJFWEoL34Cv0265hLokQjWtsurMCvdU2" +
        "xUg3I+LwWjdSMxcS4tFgTb4vQRHj9hclDIuRwBuZe5lWDa/u0rxHV+N5SXs0iSckhN6x7O" +
        "lYTv5O31q+Qa2sCMUYDu/SU+5s0J0SARON3IBi95WpRIhKcU5gVZ7bIxl5VgcMP2MLXLDi" +
        "vn4V/JQzWEE4dMThj4vfJqwftYs7t0NZa7Akpm2Qi8Ry6l0zmLfL3l5775TxGz7KySHBxZ" +
        "gCqqL2W3eb9X6WVTQcZ2nA8ULjR6z8KBxmVQIDAQAB",

        "O=Certplus, CN=Class 3 Primary CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAt5QwbtBM2X5eYOVvuybKUm9rbI" +
        "WZpQvvABpvG01YtRgH+t17+MssUt38nLiNUum5sgt89Y9bmUgJWN7NSJWGJYkFNCcwC1e2" +
        "DHdjKctsqj65mVESDZhwdkdM+UmWIgj5a+qqADajFaccHp+Mylp5eyHaiR9jfnmSUAkEKK" +
        "3O420ESUqZsT9FCXhYIO+N/oDIBO0pLKBYjYQCJZc/oBPXe4sj45+4x7hCQDgbkkq9SpRV" +
        "x1YVDYF3zJ+iN4krW4UNi3f4xIv7EMuUx+kaVhKXZhTEu9d9bQIbv3FiJhjpSYr6o97hhK" +
        "2AykriIoxqCGGDsiLHCYg4Vl3RMavwCZ8TWQIDAQAB",

        "CN=Autoridad Certificadora de la Asociacion Nacional del Notariado Mexicano",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7tlrVYRxJvaOrUG71tLeY+ryP2" +
        "XyOxPBrlEm9L94j8ZMSay/Qd71KMco55/XgOXU7iMrk5U9yY9q9coA6RDHiIIabqNf8DRS" +
        "ISVoKPiV8ICVoiyxP2r2KNbihP0WZ5wluXXb5cZZA7SrQgeI1VxIRaIJA8muZ5KoolPHyq" +
        "t+mhKVWgVXjRBklicRsOYyMFvNPQygGxMtuxqr3TnOkmuiBNQTX213Z1Q5qHtpisZfeMoH" +
        "GGlu+cDT0IqOrx4waO742KhmDIR9I2qJPGJNFHSs25uc/LCD/gcw8factEjI5jpCJQko91" +
        "bCsdejmHcCh+qKwV3axIonB4VeSExVKEDtCQIDAQAB",

        "O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhA" +
        "wL0TPZ2RHP7gJYHyX3KqhEBarsAx94f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lw" +
        "dd8KkMaOIG+YD/isI19wKTakyYbnsZogy1Olhec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpw" +
        "IDAQAB",

        "C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhA" +
        "wL0TPZ2RHP7gJYHyX3KqhEBarsAx94f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lw" +
        "dd8KkMaOIG+YD/isI19wKTakyYbnsZogy1Olhec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpw" +
        "IDAQAB",

        "C=FR, O=Certplus, CN=Class 3P Primary CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqzf/62CbQXhp9UlYsN4fcWmmK+" +
        "OuUMapvJPpIL7kxBOCVu/wQzIJypt1A498T+HgT3aeC61kehQ6mp2/LxYLZRyp7py84xpl" +
        "y0+F6pJWdWbWVUDv+8zWOD+rHO9CjRmJ9reVhsKnHen3KfEq2WV5/Cv1jsoad36e6Kz5Zr" +
        "9F++gTnV+2c+V9e477EnRdHwZehRumXhhEALq8027RUg4GrevutbTBu7zrOA9IIpHHb9K4" +
        "cju6f8CNbLe8R3MhKoX/rNYoohnVl2o6uaxtRezmTcPbqF3FXYKYrEpaquYrCAwQdLxi9j" +
        "pJBGbYURwmpth1n5y/rmBRPVy8ok97iWfNUwIDAQAB",

        "C=FR, O=Certplus, CN=Class 3TS Primary CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvWWaI0MAPPklAUOYW0Y0N2c39F" +
        "tXjqPezYwvQbMVPeWYi/LMXKfHrzXHs6dPxxApV+kDiYNyBnZSwXACN0Dt8M6LsbGJrAKo" +
        "W93c1UNFBtwotulRG2ru83tIxZ0Rro2mcpPAJUKRqD5G4mhMgUCwQtN6vntH0kdQDKQSps" +
        "rkEtDAfDo8AanKApbeglrF+xm6PJzYD3QfmBiulFAyB1IQEUpL7FhVLNSeS5R7BdJy3wbw" +
        "jcsInuTutEStgvEbYWrxs/gWMTZCJLqQv7V+YW7CWQxUebRMiCgezBvfhIsjyL6vB/KRst" +
        "qNyoxffCg8fIlsBlm9Ps7FgtNqyaxoVe7FrwIDAQAB",

        "C=US, O=RSA Data Security, Inc., OU=Commercial Certification Authority",
        "MIGbMA0GCSqGSIb3DQEBAQUAA4GJADCBhQJ+AKT7gWJ7zhAn3ej3vmxuxnCZ27jVBQNpKI" +
        "Kccn+WP47srCmSP4oU+EJ2vr1dA7mQ1NC8BrJRM1/Ewr+2i4+ZtmIiYN3b3yCCtMqiLy1Q" +
        "7ZQy3uBVjdRo4uBM0s0FFi6VZlxhUjgeUaiCocTvJekK5osrjjFm2fjZ/b07adnrAgMBAA" +
        "E=",

        "C=DE, O=Deutsche Telekom AG, OU=T-TeleSec Trust Center, CN=Deutsche Telekom Root CA 1",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQ3ZsMoBdERA+vIUBzZ1bwPmloEbrZN/" +
        "KBrsMkrGmhzfymGFVW/4ufMsHb53gsOdtggUGl79PNgI0YPOJSDAuf92Se5aDwuGFi9L/g" +
        "o9pYK/0VBGu9Op58nfI92OSVw+xOwvFlqwxL7EeCW+LhUHXY9mG0GFztM6BLHoP7T4S8eQ" +
        "IDAQAB",

        "C=DE, O=Deutsche Telekom AG, OU=T-TeleSec Trust Center, CN=Deutsche Telekom Root CA 2",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqwujNeCLKRSxFIWvPBDkOW81XU" +
        "qu3ephjZVJ9G9koxpgZqSpQCKE2dSl5XiTDmgBrblNXDrO07ioQkDfz6O6gllqkhusHJra" +
        "CCslJ/lpI0fx4Ossepv1EwLQfjR8wp48AFmr9doM9TI8K6xQ2tbD3oOUyqgMmTIOCEhWW2" +
        "r72uFYWAFJX3JBPBUGAY5draq4k7TNnuun6GotUjTbOu9cdVHa2/Mx+e5xmDLEVBVEDPmb" +
        "Ve2t3xgIoKOGiknuUwWPGUzV3lh5m9JqHEKrxdWnz2gPluThYZh2YciRfNY+AOKRUIfhnQ" +
        "rmrZfSHcY6fcu82gM01Y5bAfVqB7cWtm5KfwIDAQAB",

        "C=US, O=Digital Signature Trust Co., OU=DST (ANX Network) CA",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQC0SBGAWKDVpZkP9jcsRLZu0XzzKmueEb" +
        "aIIwRccSWeahJ3EW6/aDllqPay9qIYsokVoGe3eowiSGv2hDQftsr3G3LL8ltI04ceInYT" +
        "BLSsbJZ/5w4IyTJRMC3VgOghZ7rzXggkLAdZnZAa7kbJtaQelrRBkdR/0o04JrBvQ24JfQ" +
        "IBAw==",

        "OU=National Retail Federation, CN=DST (NRF) RootCA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2aybd/pQ08zcuUCsuXJqAIcj/A" +
        "+WIdAmr+TitV/606Z9ITAuzBeCj5h0/Gekpt+Il6JCKfWn2xGT+14jMMKqvCLnQRvl7SXe" +
        "yD/b3ldFeEBGg7LVGj3fD0Vt1WMCddgvxm6rlZF0Nw3LTQlc0dRbOtrdDshrmdjVOczfhV" +
        "XEklMCo+H3gMlwo9rcM8R/okcIHDWWH6EDHDCD9MTM/5jDsEZEosC/rdvSgfZMmCynXiTz" +
        "hspj1bp98JrAStAbWO7sqWfPaQJsIsBgLCzRyCDqyC373Zy7y1FM3OdXBDtUmxGlMnTsdA" +
        "HzkBVbL3wsk2W5Zme0gYg15Z6RGH+BqEHIywIDAQAB",

        "OU=United Parcel Service, CN=DST (UPS) RootCA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7xfsrynm2SsnwNt7JJ9m9ASjwq" +
        "0KyrDNhCuqN/OAoWDvQo/lXXdfV0JU3SvbYbJxXpN7b1/rJCvnpPLr8XOzC431Wdcy36yQ" +
        "jk4xuiVNtgym8eWvDOHlb1IDFcHfvn5KpqYYRnA/76dNqNz1dNlhekA8oZQo6sKUiMs3FQ" +
        "UZPJViuhwt+yiM0ciekjxbEVQ7eNlHO5stSuY+e2vf9PYFzyj2upg2AJ48N4UKnN63pIXF" +
        "Y/23YhRtFx7MioCFQjIRsCHinXfJgBZBnuvlFIl/t8O8T8Gfh5uW7GP2+ZBWDpWjIwqMZN" +
        "qbuxx3sExd5sjo9X15LVckP8zjPSyYzxKfFwIDAQAB",

        "CN=Autoridad Certificadora del Colegio Nacional de Correduria Publica Mexicana",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmO0dhYH7/jd0viOAJ18bQX6856" +
        "WK2HNdlkjqq1iqfaUdz/4gCtnydQnts9X9+JMqGaleqLEU8tZChkFBXk/FVqeaokJvLihI" +
        "6i6r2cHZmvClnotdEWeaNzdTYGbxIv93d0fp3dwYRu4u3+LBluDqWN6H65OIaZmwPm52KU" +
        "Bhwyhmc3+sMXb0OM3WMo9zMhAVNNJ8RND8eQwAnX0P4+P3RPWedEknrRvXMshTrm8qsNe1" +
        "LRgsbjs6TUzb9Wi1L7AMkPk93HU2msLgv7uWiMJr7hjXTlA/V4tnaKS+AzNdWRI0if52yN" +
        "kVdgFUZP2s41DvEMjQ7l/sHd9PBZg8tBReAQIDAQAB",

        "CN=DST RootCA X1",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0sYmtuelPcHEaNVQb1PFb0kTCb" +
        "ivLEiNFGqjF19a+dMudS/YKGLRky/8TdSrh+UIx5nnkj91vesltBXBmxk90kSN13QgbTcC" +
        "j2mTW4rEGZ30sg78Fmy5sQWSg9GFLGCUPkVVoNmrCCHmYOg7dPKZUFFo0AMtsYC+o9hSsE" +
        "TNQ0pwjliFleFOLNYtQW/WhOfImETKR9ssJKVpJs9ruCdiw/TJepIj7RNngq5FLkXlfnI/" +
        "hZ2UYhDmPJGhrXcA4BXs84SAcnqObmCXxyRZEDSDW+GlpGm2VzUceFnG0y86c2fulMoEEw" +
        "ViBnAjs/R87kXZZAtbSaqkQ84mxEQSbLjdeQIDAQAB",

        "OU=DSTCA X2, CN=DST RootCA X2",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3HXwjMB1lprAYh8m98ThmurgVn" +
        "Nbmc0BRKgIttWn2hoEGDmSSnijgcL1d3pQtHD/mqvGx8pug09CmPsmC9rcbdapmVVSZ+ko" +
        "A5Lc5bAFmg8V+WtZclby+jn8qmjuDx8Qgy/8nfoXlt2C4+ZFfcBLgEQf7SzghP2RXJJUaS" +
        "XlYmnc5e4AUr0zC611AoWnZFAtxRkZMMAm28nT/S6ZrVm1C03UQa6FSENZ3Leo4qLew4/X" +
        "uKFipmhQUuTPMaeUhdqfRjIXVuXy62Y9Ev9D25jvd8/LgY00scZQSibR5D5BUK9sriI0Lt" +
        "VrboO6ebh2ZUjaCSlkYyK5+0d2hYyGRMsJ2wIDAQAB",

        "C=US, O=Digital Signature Trust Co., OU=DST-Entrust GTI CA",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQC2HfdLjQ8T4xL1Cf4GMg6vTEH1fdRHPS" +
        "oK34MF3t595gMW9lE6y0caSq1+xP0dtL50injdC4OOtIQTxPv4bSmuoeEPD0PjtV5gafqD" +
        "lPx55tx27dFEK479Erv+F3cXDIntp+9RfcTtOMM7o3r74k2gYLXy/RNl08bsP741nD0i7w" +
        "IBAw==",

        "C=US, O=Digital Signature Trust Co., OU=DSTCA E1",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQCgbIGpzzQeJN3+hijM3oMv+V7UQtLodG" +
        "BmE5gGHKlREmlvMVW5SXIACH7TpWJENySZj9mDSI+ZbZUTu0M7LklOiDfBu1h//uG9+Lth" +
        "zfNHwJmm8fOR6Hh8AMthyUQncWlVSn5JTe2io74CTADKAqjuAQIxZA9SLRN0dja1erQtcQ" +
        "IBAw==",

        "C=US, O=Digital Signature Trust Co., OU=DSTCA E2",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQC/k48Xku8zExjrEH9OFr//Bo8qhbxe+S" +
        "SmJIi2A7fBw18DW9Fvrn5C6mYjuGODVvsoLeE4i7TuqAHhzhy2iCoiRoX7n6dwqUcUP87e" +
        "ZfCocfdPJmyMvMa1795JJ/9IKn3oTQPMx7JSxhcxEzu1TdvIxPbDDyQq2gyd55FbgM2UnQ" +
        "IBAw==",

        "CN=Entrust.net Certification Authority (2048)",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArU1LqRKGsuqjIAcVFmQqK0vRvw" +
        "tKTY7tgHalZ7d4QMBzQshowNtTK91euHaYNZOLGp18EzoOH1u3Hs/lJBQesYGpjX24zGtL" +
        "A/ECDNyrpUAkAH90lKGdCCmziAv1h3edVc3kw37XamSrhRSGlVuXMlBvPci6Zgzj/L24Sc" +
        "F2iUkZ/cCovYmjZy/Gn7xxGWC4LeksyZB2ZnuU4q941mVTXTzWnLLPKQP5L6RQstRIzgUy" +
        "VYr9smRMDuSYB3Xbf9+5CFVghTAp+XtIpGmG4zU/HoZdenoVve8AjhUiVBcAkCaTvA5JaJ" +
        "G/+EfTnZVCwQ5N328mz8MYIWJmQ3DW1cAH4QIDAQAB",

        "CN=Entrust.net Client Certification Authority",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDIOpleMRffrCdvkHvkGf9FozTC28GoT/" +
        "Bo6oT9n3V5z8GKUZSvx1cDR2SerYIbWtp/N3hHuzeYEpbOxhN979IMMFGpOZ5V+Pux5zDe" +
        "g7K6PvHViTs7hbqqdCz+PzFur5GVbgbUB01LLFZHGARS2g4Qk79jkJvh34zmAqTmT173iw" +
        "IBAw==",

        "CN=Entrust.net Secure Server Certification Authority",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDNKIM0VBuJ8w+vN5Ex/68xYMmo6LIQaO" +
        "2f55M28Qpku0f1BBc/I0dNxScZgSYMVHINiC3ZH5oSn7yzcdOAGT9HZnuMNSjSuQrfJNqc" +
        "1lB5gXpa0zf3wkrYKZImZNHkmGw6AIr1NJtl+O3jEP/9uElY3KDegjlrgbEWGWG5VLbmQw" +
        "IBAw==",

        "C=US, O=Equifax, OU=Equifax Secure Certificate Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDBXbFYZwhi7qCaLR8IbZEUaJgKHv7aBG" +
        "8ThGIhw9F8zp8F4LgB8E407OKKlQRkrPFrU18Fs8tngL9CAo7+3QEJ7OEAFE/8+/AM3UO6" +
        "WyvhH4BwmRVXkxbxD5dqt8JoIxzMTVkwrFEeO68r1u5jRXvF2V9Q0uNQDzqI578U/eDHuQ" +
        "IDAQAB",

        "C=US, O=Equifax Secure Inc., CN=Equifax Secure eBusiness CA-1",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDOLxm8F7d33pOpX1oNF080GgyY9CLZWd" +
        "TEaEbwtDXFhQMgxq9FpSFRRUHrFlg2Mm/iUGJk+f1RnKok2fSdgyqHCiHTEjg0bI0Ablqg" +
        "2ULuGiGV+VJMVVrFDzhPRvpt+C411h186+LwsHWAyKkTrL6I7zpuq18qOGICsBJ7/o+mAw" +
        "IDAQAB",

        "CN=Baltimore EZ by DST",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvMyzPUN5uEf5FbduJrFMkph57c" +
        "Vw8zrp1d0D9Co/YIyW5UcWAvc2svGeJoj1nkJlng+uf+PMsW4h9fGIInTWH7J3BDkyuke1" +
        "NcATXQFyowVDzE7aJpqHqGFj9GanwxVG6tHR6jDDu3Fqm8FDhsE5H8ZWYAIb/Ig6oJm7jN" +
        "d4YdBeV4+RO4CLbv/JZYEKObuQEyA1SD+l4b8twXGDhSDtIIfLtv4ZjATd7Sld3woSzolW" +
        "8h9aGTFYtv1jNurJI96nkZcnZXKZbMd6RMRfvpsfHsqeWBymqiNq4wYbkiTYVyIJUBWQRv" +
        "CDXraATBKBPWZvBFU6iGvQ71aHUKC51lUbnQIDAQAB",

        "C=US, O=Equifax Secure, OU=Equifax Secure eBusiness CA-2",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDkOTmTHlIGGyg2+LKjKcXtjrIRvf7r57" +
        "R0wo//BefZnQa/Esg/DvLW0SSyEd7RcwmK1LEsmAkNHlBGsoOmRY1iaLuFGyBwMqpAzaaW" +
        "X8RxNz8E87dBJDkHGh4uYVigEgvlpd/Fq+o3ccwcyDc6uZdSp6zFaiSUTpx7z8Bq1t8hvQ" +
        "IDAQAB",

        "C=US, O=Equifax Secure Inc., CN=Equifax Secure Global eBusiness CA-1",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC65xeQAmWxNFU8ScJR1d+n0TeP0eeBc0" +
        "FSYJudoRcmeK3HsegmlDK13jONOi/b8pp6WnOYo1zp+4pzG1znw7+AbM2p9NYrwPf5mapj" +
        "orFHAg/U5FE6EjxsilpUhHDbwcWQz3JFy6hZwM0znT+jluuFMyEcPh4+YG52nGeFxcjDYQ" +
        "IDAQAB",

        "O=EUnet International, CN=EUnet International Root CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCeQTvZQUmLKJxZFPdQaCh7TQhcZ/+FHg" +
        "umzzoyArB8fEqftokCIQxKmYvLZFF+eFq2XqlTt+/vx9+lIVmXTuIH5S18GdUqysgz05YQ" +
        "Lt2gAJ/9yuhhqVPKth0YPpwR4GPnKmdbyESV8BNVSLu+VbhnN83LABMN/E9pFGpRlOy8Jw" +
        "IDAQAB",

        "CN=FESTE, Public Notary Certs, EmailAddress=feste@feste.org",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDhg/ObLsnn4cf0VAXNdkD+tLMTvucVXo" +
        "Ym6EB3GlU/0QMmjPqHX6TF+f61MonGf0GR2BVATnBS8PHa+GI1mV4clFNhzD5iwINdWNH4" +
        "SBFxbPewd+EYl7QHKDCRMcdPVPOEnsxZiUVtfrTJ245ClWbU3x4YTfylD9YahDnEyvK98w" +
        "IDAQAB",

        "CN=FESTE, Verified Certs",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDqY58fOBqEBISzS5MZhKJ7YsOnqyzsYE" +
        "5VEeIEMicgNfkaeB8nZ6fggrAF6Capm4pEVr9LhFOjIqYOFlO5f68QyDMYVNnGTHzRW1ZS" +
        "U4amWz8T8sMB0jGhM1y8XeTcYjzKI5dPcPuBjrDZnq+T6raxJI0ELVFDPDjsJ0Nxh+g8xw" +
        "IDAQAB",

        "CN=First Data Digital Certificates Inc. Certification Authority",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDfHBQeCbm/pEByIJl5toQi9NeFksUEJO" +
        "gHLgLkF5UFN5V2Pfyx5Q+HDmK5LDCXJuELFWcAphXe6I3LlewCWFLAR2UzTFafCh8EwDdQ" +
        "gVe63/rya2fry9CAD9lXlRBlewZFWOuutF7jkxUrmby2KS/7Qp9HKy5M6zQoMpkO7/9voQ" +
        "IBAw==",

        "C=ES, O=FNMT, OU=FNMT Clase 2 CA",
        "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQCYP60ZNpM9Pv52QhT9NW/x+q0ieljjRt" +
        "Bdxlr5Yi2PMV7+tDD+UHSs1p0d4GLGSd0UEn1xC6wGwT/XBofgkInW5eMDsvInsZ8zyKpr" +
        "NkqjxD95QZ2JRi8rPmPUOFaRqh2xDUJ1TfOHTuMPTcy0bL9iE4fq0JuOtuL/GfSUCdWWYQ" +
        "IBAw==",

        "CN=Belgacom E-Trust Primary CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCq2bmz1U9qTcVB0HsEYWqLcYEH2mTjWG" +
        "4nVcKtzhew/PqSjQjwHHL/ssMx/uBqh5dMzENXpyh5OrWDXaQdavFqxT4UIh1ZBm/wpjF3" +
        "3LBJOObLDA/+qnI0iNooOiFa7nQrG6TbWxMWtXNfw66M0sA+PbDL8OyLhgvCwUQYWmOo1Q" +
        "IDAQAB",

        "C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2g7mmY3Oo+NPin778YuDJWvqSB" +
        "/xKrC5lREEvfBj0eJnZs8c3c8bSCvujYmOmq8pgGWr6cctEsurHExwB6E9CjDNFY1P+N3U" +
        "jFAVHO9Q7sQu9/zpUvKRfeBt1TUwjl5Dc/JB6dVq47KJOlY5OG8GPIhpWypNxadUuGyJzJ" +
        "v5PMrl/Yn1EjySeJbW3HRuk0Rh0Y3HRrJ1DoboGYrVbWzVeBaVounICjjr8iQTT3NUkxOF" +
        "Ohu8HjS1iwWMuXeLsdsfIJGrCVNukM57N3S5cEeRIlFjFnmusa5BJgjIGSvRRqpI1mQq14" +
        "M0/ywqwWwZQ0oHhefTfPYhaO/q8lKff5OQzwIDAQAB",

        "CN=GTE CyberTrust Global Root",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCVD6C28FCc6HrHiM3dFw4usJTQGz0O9p" +
        "TAipTHBsiQl8i4ZBp6fmw8U+E3KHNgf7KXUwefU/ltWJTSr41tiGeA5u2ylc9yMcqlHHK6" +
        "XALnZELn+aks1joNrI1CqiQBOeacPwGFVw1Yh0X404Wqk2kmhXBIgD8SFcd5tB8FLztimQ" +
        "IDAQAB",

        "CN=GTE CyberTrust Root",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6jr11kBL65Xl0stn3JtQOQR3pNgdWct" +
        "W4adpU1LHWeG2q4zs9o4Q3JcevrwTcsyKx6W2+gm3rjS+9tK5wHqLWbiAxUeZWXHNSsiNQ" +
        "Trz7mmdAxIYRRsdDIrrqAE9scs1hnN7L+u4w0ub6W53Fmdwg+Dm/ZIwHVju93Gxe9r/h2Q" +
        "IDAQAB",

        "C=US, O=GTE Corporation, CN=GTE CyberTrust Root",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC45k+625h8cXyvRLfTD0bZZOWTwUKOx7" +
        "pJjTUteueLveUFMVnGsS8KDPufpz+iCWaEVh43KRuH6X4MypqfpX/1FZSj1aJGgthoTNE3" +
        "FQZor734sLPwKfWVWgkWYXcKIiXUT0Wqx73llt/51KiOQswkwB6RJ0q1bQaAYznEol44Aw" +
        "IDAQAB",

        "OU=ValiCert Class 3 Policy Validation Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDjmFGWHOjVsQaBalfDcnWTq8+epvzzFl" +
        "LWLU2fNUSoLgRNB0mKOCn1dzfnt6td3zZxFJmP3MKS8edgkpfs2Ejcv8ECIMYkpChMMFp2" +
        "bbFc893enhBxoYjHW5tBbcqwuI4V7q0zK89HBFx1cQqYJJgpp0lZpd34t0NiYfPT4tBVPw" +
        "IDAQAB",

        "OU=ValiCert Class 1 Policy Validation Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDYWYJ6ibiWuqYvaG9YLqdUHAZu9OqNSL" +
        "wxlBfw8068srg1knaw0KWlAdcAAxIiGQj4/xEjm84H9b9pGib+TunRf50sQB1ZaG6m+Fiw" +
        "nRqP0z/x3BkGgagO4DrdyFNFCQbmD3DD+kCmDuJWBQ8YTfwggtFzVXSNdnKgHZ0dwN0/cQ" +
        "IDAQAB",

        "OU=ValiCert Class 2 Policy Validation Authority",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDOOnHK5avIWZJV16vYdA757tn2VUdZZU" +
        "cOBVXc65g2PFxTXdMwzzjsvUGJ7SVCCSRrCl6zfN1SLUzm1NZ9WlmpZdRJEy0kTRxQb7XB" +
        "hVQ7/nHk01xC+YDgkRoKWzk2Z/M/VXwbP7RfZHM047QSv4dk+NoS/zcnwbNDu+97bi5p9w" +
        "IDAQAB",

        "C=hk, O=C&W HKT SecureNet CA Class A",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtBuiCqVMc2NGUUh0Y6i0jBbb9M" +
        "hn3qFIAv/Lo8+n39mxMeDjLihxBKZkWsZc/tCnuOo+Ctr7EX9/JCheyIqsbniqyKIYOZ5M" +
        "UNHwmLXvpLIbYGu/+XO0C3X5Irvp5YGgldJ2THzTp/5dlRXtB9TH3mAwAO7yLpTxhjLlWV" +
        "Ho34CiKgDvPIhdEeMAX1TkDEcQbLD1+DN2HDRmW9S7NGM502aUOuzNIinz9hK71CEpN6VE" +
        "Td+JDAQMfUF7h/MWwUMpZLTWRWerhkxljwG36mOMTnhUREcaU4aMaxgnIQvFVmYOJfbgea" +
        "xoAHTpmmQ8SU6e4B3IiBtQBvddCfiNixP9XQIDAQAB",

        "CN=IPS SERVIDORES",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCsT1J0nznqjtwlxLyYXZhkJAk8IbPMGb" +
        "WOlI6H0fg3PqHILVikgDVboXVsHUUMH2Fjal5vmwpMwci4YSM1gf/+rHhwLWjhOgeYlQJU" +
        "3c0jt4BT18g3RXIGJBK6E2Ehim51KODFDzT9NthFf+G4Nu+z4cYgjui0OLzhPvYR3oydAQ" +
        "IDAQAB",

        "CN=Microsoft Root Authority",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqQK9wXDmO/JOGyifl3heMOqiqY" +
        "0lX/j+lUyjt/6doiA+fFGim6KPYDJr0UJkee6sdslU2vLrnIYcj5+EZrPFa3piI9YdPN4P" +
        "AZLolsS/LWaammgmmdA6LL8MtVgmwUbnCj44liypKDmo7EmDQuOED7uabFVhrIJ8oWAtd0" +
        "zpmbRkO5pQHDEIJBSfqeeRKxjmPZhjFGBYBWWfHTdSh/en75QCxhvTv1VFs4mAvzrsVJRO" +
        "rv2nem10Tq8YzJYJKCEAV5BgaTe7SxIHPFb/W/ukZgoIptKBVlfvtjteFoF3BNr2vq6Alf" +
        "6wzX/WpxpyXDzKvPAIoyIwswaFybMgdxOF3wIDAQAB",

        "CN=Microsoft Root Certificate Authority",
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA8136gGfUWqepDCyQINA1CDx1hM" +
        "23B4mcidrezsNg+pFoWp6UcSkYdnzC4MgldpQOWPoENDbm36/3gLrpWAsrk+WdBeN3IpH3" +
        "NGQ8IpEdXuEJkLwU/vx1WBnhebcHkqOuiFkI2J8HygNY/GgpbTLX0qjLS/zhC0gyT+bruK" +
        "1P5FxvE5SZ25XVdduoGreUkbR3W/VIDI9qeX0UcAR9ba+Q9dpw2Ee3v5svbOcFt+ERYKx5" +
        "kRR8xdam5OF+1cN+5ZLSPAC1NoLeeeFt87Vu+J8zyctSfXOYNtuLoWuilZebo97CTSb/Bp" +
        "ZnJQbI56zk7hIzlTGZyDUITjTKeVPVtb5jMllANsClTgRNPdtbBzPkWL/vP1Nk2EJZNVf9" +
        "D0V8JARNntY4dBGXIpDOaER0km/VS2+whuPHNkKg0PzBwFr5o2G5MEdxlgoWsJHAQpXvEH" +
        "8oauMqH7HkzQM/d3EExyD8SQ8dRYik18t+iK2OLexF28RRBMkq/OyGnpoRl1vezlOI5uK3" +
        "/ayVwihA2+8EkN+BMznZskWlI4cGpVWJMbsGLWAOQRh9Hy61l8sR6xXVJKWU7xUUif1Lc/" +
        "oyW/zRMwD5WWJwBzLqLqtALXvK3SFnGzCZjxaqI6hB0bBuEZs2xN5AdJzhWGXBYB56WzjI" +
        "j7sEJnzUFkDltmtsqob9AL/OwTUCAwEAAQ==",

        "CN=NetLock Expressz (Class C) Tanusitvanykiado",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDr7LBsYYojJa9gIOPZn/yTC9tdjbChs0" +
        "A6gs79deB4MgOGWoaVke1T+p1A/Obo3dlbegO9XfM7DMNReZutVaDp0AMQrwq6FELZUiYR" +
        "IsfSIMyCpJqp/riBdp1qt9I2dT6xhgn2bm1+Trd67K5xhPYEMwglMut0rBZExuRAkx1/rQ" +
        "IDAQAB",

        "CN=NetLock Kozjegyzoi (Class A) Tanusitvanykiado",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvHSMD7tM9DceqQWC2ObhbHDqeL" +
        "Vu0ThEDaiDzl3S1tWBxdRL51uUcCbbO51qTGL3cfNk1mE7PetzozfZz+qMkjvN9wfcZnSX" +
        "9EUi3fRc4L9t875lM+QVOr/bmJBVOMTtplVjC7B4BPTjbsE/jvxReB+SnoPC/tmwqcm8Wg" +
        "D/qaiYdPv2LD4VOQ22BFWoDpggQrOxJa1+mm9dU7GrDPzr4PN6s6iz/0b2Y6LYOph7tqyF" +
        "/7AlT3Rj5xMHpQqPBffAZG9+pyeAlt7ULoZgx2srXnN7F+eRP2QM2EsiNCubMvJIH5+hCo" +
        "R64sKtlz2O1cH5VqNQ6ca0+pii7pXmKgOM3wIDAQAB",

        "OU=Tanusitvanykiadok, CN=NetLock Uzleti (Class B) Tanusitvanykiado",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCx6gTsIKAjwo84YM/HRrPVG/77uZmeBN" +
        "wcf4xKgZjupNTKihe5In+DCnVMm8Bp2GQ5o+2So/1bXHQawEfKOml2mrriRBf8TKPV/riX" +
        "iK+IA4kfpPIEPsgHC+b5sy96YhQJRhTKZPWLgLViqNhr1nGTLbO/CVRY7QbrqHvcQ7GhaQ" +
        "IDAQAB",

        "CN=Post.Trust Root CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1n8T5A0k2Nj76bbDsVKjTty3O+" +
        "L3Dl+B5gHwpuY2cNgTc6H/UgiQ8hW88jIcqNfhBhB7QaiUxz89RBXcgFHnMP5TSPWQX21t" +
        "JeBgu6D71sYp+E1wUBo3oA7NeCq2aPOZ1AyOXhJi/8JfWporiEequ6HZdfAsXP5twrFbMc" +
        "yDhxqnvpAO6BBUU1ILnEnzgAL+byemo1cwuNu40AAEA+Tl1EMG66toTWgm0pk0ueASln9L" +
        "u2tuIXHmCEVKHWYNN8kD4dHK3LEvcPa3gWKWG2Sn/rvhhutBn6ic2Mqg4dYv+A/hukA492" +
        "3RpcpMGciW3MxJHAq206iROvna7B3Nc0okPwIDAQAB",

        "CN=PTT Post Root CA, 0.9.2342.19200300.100.1.3=ca@ptt-post.nl",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsH7iOgHxSK1T1HHO276A4FCtma" +
        "KEeto6JyQ6EYE2Eg3mo5mOpMwmtQ5hxu4oq22G3y6XYfpAacmNjMQxe/pSXlZMIJ5gGl9s" +
        "SnjJiTyflYasd2cOpg5C6CxiSTJLBD4yQ5AOCiLKyHQOhe+DgcVb8ttshQhvTialBqt245" +
        "iiTl7EgODo+8zpMGzycmGuJ35T1BWUD9KPeYLZ9o+rxhPmHJh0SwBhDnlpVPKQsqMJAWX3" +
        "BEdsTvopK/AOBheT3ILAEd6PsDBGWUhKZs42r8fPMdGSdBQj1aq64InbEtHs1GkjuAsWST" +
        "POGvninF98aB13uwGqZ+Ixxv/WOmn9DBt8IwIDAQAB",

        "CN=Saunalahden Serveri CA, EmailAddress=gold-certs@saunalahti.fi",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA5wQp3NbgUtPWTwCvHIGIvzxUcv" +
        "OeeWP9y2DaDHxyL8obqeIQaWd6OZ/CoCXMg4ONgxEcuP3n26mIowySIVfBquLqM35KZgO8" +
        "c43SHCn9x39D7Y/rV3uhQb9NczFKNyi0GFdYPGhwUJO6EB14zZPDwoLvuN8PDFjVMFdDOh" +
        "QlKjhZBrREzdvJXkbyS7gcQ0GB0j5Dsq4hnhtKgHymyrP0JqkuLPi39zwYD5sybxEJc8TN" +
        "L+jT7Ek284GN2ML/0Bpt3dgUvzLQ6cMNPgiv7dpLnWrPE4uQgmn612cjYUtb/aWAZB1696" +
        "XT2ncceLtR++dGgJBxcbYW+EO0Gb0Yq952ewIDAQAB",

        "CN=Saunalahden Serveri CA, EmailAddress=silver-certs@saunalahti.fi",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0neMvIdsNk5TqmhgRbgjd2fj7k" +
        "mC5mx/XpJdtRxhUteoYEsW+ut5fp1MkulXe16GMKkoPH030SHidhoZw++q2u74AZ4aOSov" +
        "k3UDZj9uKU2NhGOMpx8VlLQ0SbTk00GruvvEXLWecvUoyjKCY0zHRPi0HcSKCldVkK8wiV" +
        "QOp2gm00AHIrPOPKP7mNckPN58gkm0NIx9JNtkbmSy6f+GyKx+q1Pk0kH0EYTuR0wIHUTm" +
        "Vk0AfNqJQjnveAjRhea+XJ4zuTX/HM70g7XyZMUxSKm0rMXYPIwabab/Qq3z+EvOrNrFir" +
        "APAyPB9fPHWX8w8d9mHVoxBaJGHTnkVbOtDwIDAQAB",

        "C=hk, O=C&W HKT SecureNet CA Class B",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAn+AlkQ8EV8LHXLFlAmYPqP3YMQ" +
        "5vgmz5wx6w46C9OERSx4x2EnhMfsIrjIrk+dwK4JVF3+seftJE+AMVAOzEsTx6tk22lgp3" +
        "vAdg7/C3N/6J/bLYB6tS/oI/vDVnM9n7LNy1WGGiDLF9lNGohGkkPZfNmwhMUImBmh/Swi" +
        "BvzD8OZcThSEncO/nlKjEHbqZrR6gZWq7ToXS1vMLbOT36q7DwySIJ1DxGaGwuLh/4qIwR" +
        "oXY1UpLXq4gh3L3pnNn4Pt4wMUwCIi9XZrtWcjk3UJmvV9D0S9Qp7alvxtOyhpGLHRBtaB" +
        "Zk8Q5tv15n/bKOcGXnb3K8RHWrAXb/N2RFIQIDAQAB",

        "C=US, O=RSA Data Security, Inc., OU=Secure Server Certification Authority",
        "MIGbMA0GCSqGSIb3DQEBAQUAA4GJADCBhQJ+AJLOesGugz5aqomDV6wlAXYMra6OLDfO6z" +
        "V4ZFQD5YRAUcm/jwjiioII0haGN1XpsSECrXZogZoFokvJSyVmIlZsiAeP94FZbYQHZXAT" +
        "cXY+m3dM41CJVphIuR2nKRoTLkoRWZweFdVJVCxzOmmCsZc5nG1wZ0jl3S3WyB57AgMBAA" +
        "E=",

        "C=au, O=SecureNet CA Class A",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqaN8+JCzjoRM4Aq+qxIqjQw+L7" +
        "XdmxCIuWq3h3Ugt0vvIiMG6/BWMvfLLXDFA2+3wdDDZhMCvVVJh4fpLZ6l5XY2q+JkJViI" +
        "wxsbAvBdsY+fE03CUim0EDVPNoivCy2BCCRhw2iNWm0x6FQZUxf9pxP2QJmmqCnAn0J7Jy" +
        "nB7tvvjQNkJYGx/pUaHtoQQWIbVn8YGEiY0k1LwRhot2lna2RMbo8CvxRpe/ZEIxDpLrxe" +
        "Ys1bnMyjjoxRgbSiorG8qMnoKpiqu0sVoeHpkHqef+hlBegRcXpv43XeVT/L2OrIAM0llH" +
        "JkHu99ED5NL5F5vQLq15DBSWhuWRQl4t3dCQIDAQAB",

        "C=au, O=SecureNet CA Class B",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApmPZxhVadudGZcc0kfl73Va7+J" +
        "Y1LinKp30KHvcxUuhayNPPOQFOW/AfsbhK0rNHQ2Y/AUBOMEnhD/3rEmN4zPYWYhj1b2n9" +
        "fm4zdiGjwIgP6uYl/KmXzBhyxzG2C5vNwsV4YWNFrDSmJ3hoxL1SaM6ETdIkpShsgObK5s" +
        "/mmp5QeM7zNtKjQ1ocBq/LIO7QLMREGJBssZFkZbm3hYNLqJGZxeCc97hQ19OwT5rtY/tN" +
        "9NQoJDqAW3uTjMUFhK87hv6BMce2nV8a6pB7sEZesghSAFcNVVKDeJVK/WiPntlQtktT+v" +
        "KFApVOOPWDp5bUMT8/p8o3U9zFL20adKbMvwIDAQAB",

        "C=au, O=SecureNet CA Root",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApyi02Dz1v3oGkb2lQkyzfJ6IZp" +
        "nF2xfURVTDe8DwJFZmmL9E4HkTdmiu3Zp0z6Lpl+bBwKnD9yzVNjtzna+C2twOX1Ov625Q" +
        "16jwqo6rY9Kbdf5VCnzRs8BZk1Eqh2mKGe3k19eOFKu1GVizzmzgTYLTA4TBqwAYekmoFX" +
        "0IyQFgJ5To+wlgntE/Ts0To3j9ZfcRX/abADCMIu0oiWUb0x9he8Mjo+PGgPmD8/e63oZ4" +
        "X/aVw4xqSCJlhdMiefb9RBboD2EENip1xtviZRQnYtyCXJYSMw5MGNX2PJ2xzWEcsYX5A9" +
        "G69kzW7p990ZIh8PYKFqQ0h/dWj5O+l69SpwIDAQAB",

        "C=au, O=SecureNet CA SGC Root",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAp1uxDYpTIbpSiDiQQmVE/Vbrc8" +
        "WF8wYx5Qj8jLHVescLIwq8WgkiAfinwN5XdDGLrTbMXnP39kTwMcr1LKIF8wocMHqGM+JG" +
        "U/Zk1kersVOUY3fEYtMvC+pfsHUCXvgrzybz3tKt62V/vC5BhPyZmumBG6ecZsf49bKEGy" +
        "B1ciHHhP8CRswPpmmFfVkh1Q6nXVYVT8wfQSx/Zhuv691Bo+yp5lZK/h6nxFwiny/gC3QB" +
        "cMhzgwoHpGie5FEOjXQxL6LG2ggQK+8lPmyGtUbnl4PAq96wrgYa58j7736tjrCaRfGb9b" +
        "HoMbtkAL9/kWbNqK+V6hM6Akxb68CT5EH8rQIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA1",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlJAMS3EpHNr2aHl6pLrn0syNr+" +
        "hHkJkfxirql2PoH84XV8Yas6jHfIftNTWAurpubb4X/swtG2zvigBJFuHuBl5KB12rPdFQ" +
        "uJFG1NTaFdiUXA7K19q/oPdJPMi7zuomgQoULZwNN0VrQcpXizjwJh8x/M80jo93wT/jq1" +
        "Q8J7TOMkxVE2L8/joWJc8ba6Ijt+DqAmm79yJxbXwLGZOhl5zjkWkfaOQvfRBtj2euwRCi" +
        "sF5jSpf35niprSa7VMnftO7FntMl3RNoU/mP6Ozl3oHWeD7uUEC0ATysFcGCOy5/8VIni3" +
        "Lg59v5iynDw0orM4mrXCoH/HwjHitPCCL+wQIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA1",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlJAMS3EpHNr2aHl6pLrn0syNr+" +
        "hHkJkfxirql2PoH84XV8Yas6jHfIftNTWAurpubb4X/swtG2zvigBJFuHuBl5KB12rPdFQ" +
        "uJFG1NTaFdiUXA7K19q/oPdJPMi7zuomgQoULZwNN0VrQcpXizjwJh8x/M80jo93wT/jq1" +
        "Q8J7TOMkxVE2L8/joWJc8ba6Ijt+DqAmm79yJxbXwLGZOhl5zjkWkfaOQvfRBtj2euwRCi" +
        "sF5jSpf35niprSa7VMnftO7FntMl3RNoU/mP6Ozl3oHWeD7uUEC0ATysFcGCOy5/8VIni3" +
        "Lg59v5iynDw0orM4mrXCoH/HwjHitPCCL+wQIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA2",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlnuSIz9g3wk8WIAI42MJl+jkC3" +
        "Vh1M0Oo/LjHkO6g/+6gVwvyN6Qi0wOLyn5B9aOs6Yor4Iqe8K0Zkxx9Ax0GrjbGuhoN6n5" +
        "oaJuHCjNbCY8jyoznp3LtHnE2WQ9lcYzqEf75QcJ3PZtuCVCTMP7Su1bLtQHqOWTECSTWG" +
        "59wdAez+kp19C8X0zwFRbD2MLO41sXW5SLKGsUZyQ79FLsDW58TrSZAtvJ8w+CqwH0jN4W" +
        "cMa8Fwdh/xFAhOosG3o6sANhB6qWjdDauYOO5J1RaXVxZIG0iFXcEIPOLaX1MJZhLjsK/I" +
        "dfnFyCdRMe05jR7cntchYcDAbcWSB+8F3v9wIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA2",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlnuSIz9g3wk8WIAI42MJl+jkC3" +
        "Vh1M0Oo/LjHkO6g/+6gVwvyN6Qi0wOLyn5B9aOs6Yor4Iqe8K0Zkxx9Ax0GrjbGuhoN6n5" +
        "oaJuHCjNbCY8jyoznp3LtHnE2WQ9lcYzqEf75QcJ3PZtuCVCTMP7Su1bLtQHqOWTECSTWG" +
        "59wdAez+kp19C8X0zwFRbD2MLO41sXW5SLKGsUZyQ79FLsDW58TrSZAtvJ8w+CqwH0jN4W" +
        "cMa8Fwdh/xFAhOosG3o6sANhB6qWjdDauYOO5J1RaXVxZIG0iFXcEIPOLaX1MJZhLjsK/I" +
        "dfnFyCdRMe05jR7cntchYcDAbcWSB+8F3v9wIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA3",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmV4egJZmI2TOnIwAPgqvCOm4BO" +
        "CEuG1TdU02qLXg14xOYFW2A5ebWhqn87o92ZqUMXZ0I8n37BJd2CDUHekbojd2BA8+rBZp" +
        "O+H/EC9WJeQzUBMJzE4Oq/Dkddtx1fxKze3bDzUFFdWwZntCeyblWeK1x8Cyx6FD/Q8vC4" +
        "MlJVeBu7vRNTB0kZCyj59o1dJDt7JFqSPAVtiHEtNz/stZ6q/85x9eVEUcqm2Vk2JHQkFe" +
        "T+s2Bw4oeFQKfMDDJBOGAwK5rHaSSlrdxdzs+LPbK7UbNud4gkyVfiBWsnUcfZfvf5Q4Ka" +
        "IA4tHqseM0NjFAWLiqt86BGgwXgQ3967jTvQIDAQAB",

        "C=hk, O=C&W HKT SecureNet CA Root",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtBiikFaM1l2/RliRJ+qddeCk66" +
        "JQcIdFSUmSa7c5AEt7qNpA4eYNouA3AUhNznLhXJPTw/mSDSTvSM5HKsutkjqq1pWy8hme" +
        "PpV8j2ACdJMWKGn+O+5deJMcejwj6WE5bMUwLR+EkgVx53TBQkfpMLGjFww2Y89Q0DKoh6" +
        "VAYhQROPvOL40zsIvpjnD7sJ7HXQPu9uWNcjzIvFSSz8qQ38jbrwXx61DK0QWsBbQBFZb1" +
        "6zihafeDQ+g8pl2lLLokFi/7DjJwphLWmTb3axuj5/zHG8jYL3XRNbPpwtaPBB3BtX4EOz" +
        "iJ5KMj8P3KvczrnRcGFXLt0Ob71m+z8Z0+uwIDAQAB",

        "C=JP, O=Japan Certification Services, Inc., CN=SecureSign RootCA3",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmV4egJZmI2TOnIwAPgqvCOm4BO" +
        "CEuG1TdU02qLXg14xOYFW2A5ebWhqn87o92ZqUMXZ0I8n37BJd2CDUHekbojd2BA8+rBZp" +
        "O+H/EC9WJeQzUBMJzE4Oq/Dkddtx1fxKze3bDzUFFdWwZntCeyblWeK1x8Cyx6FD/Q8vC4" +
        "MlJVeBu7vRNTB0kZCyj59o1dJDt7JFqSPAVtiHEtNz/stZ6q/85x9eVEUcqm2Vk2JHQkFe" +
        "T+s2Bw4oeFQKfMDDJBOGAwK5rHaSSlrdxdzs+LPbK7UbNud4gkyVfiBWsnUcfZfvf5Q4Ka" +
        "IA4tHqseM0NjFAWLiqt86BGgwXgQ3967jTvQIDAQAB",
        
        "CN=SERVICIOS DE CERTIFICACION - A.N.C.",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsiov7CtZakOTiUYqiuXs+gX64s" +
        "jeQWuvA9sAWu9IN89XifvdyZIQ3ncDlRyQPse2ZyU7VZjv2Tz+JuSKO0SpdDeDCncndLip" +
        "ca3dlxPSyqIuuLqdyb5Z6Nly8oqFZhxHXrSHgtYP32cmpr02sfNdkFBRdjIsOy+qX2Fe41" +
        "TVEl3/DY0Rx4J6Nt/hTBbEdN0tau/QsfAzp/6/N2dDEi55SpSvhPsHEQhOMJN16QFUzsXe" +
        "FIbwrq6bciUPRHfi82yveZwuSceemHYyFpq8AN7gtCAFkRfdgBUU7jZBxCGP7tkAShnGcW" +
        "GlEV0AO+SndGw6Sm6D4HoxXCFl+AiHQodn5QIDAQAB",

        "CN=SIA Secure Client CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDS/LBAYGpmY1Jm5mkJcY2BmB4dHfPgSQ" +
        "3IK2/Qd1FFxZ1uo1xw3hV4Fh5f4MJi9H0yQ3cI19/S9X83glLGfpOd8U1naMIvwiWIHXHm" +
        "2ArQeORRQjlVBvOAYv6WpW3FRsdB5QASm2bB4o2VPtXHDFj3yGCknHhxlYzeegm/HNX8ow" +
        "IDAQAB",

        "C=IT, O=SIA S.p.A., L=Milano, CN=SIA Secure Server CA",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA28ELzCfTEiIuuWQWdKxZJ+IqkA" +
        "CSntWYXCtRbhsTb1RvShCihethC+ztnH7Of2WTbsxsQZzILarGs5v7THCcEXXzcom6iQCt" +
        "xy5J53PagLIs/vKXmfQCGzQvOaqL5u8F/Ln1ulR/ob+OHkg2Mwl0Yac9x5skx8OJzcpOKD" +
        "EjBhxiFY7fTxtrLUri9LDczvOQ/XmBE8E+Lma8+SJNCy9iM42oK+rpb3OnN5QEL+leTQ3p" +
        "7XwyP3lK5jp2KSBQ84+CRHJsMDRIWKpdGz8B6yHs6n6oK4Rd9sExlU8pe7U1t/60BlewFN" +
        "fyVVmMupu5MT/lqqrvJXCVkjZB8VWfwQhEnQIDAQAB",

        "OU=Public CA Services",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwOeC2xUTrnnCtF+SjyO8uvfG0Q" +
        "Cv1lRp8V2mYvhh0Zzeyjss6VwWJzTmuNHKdO8leGRt/hzoiXMxU2dnhsStamjnClZEgzpY" +
        "R4l3Gtpv8vkHQMk9Ae9q0dlrhJ7FaytOtyz4pGpXq2gxuhlmuuwbV/vOStZLeMPBgT1Llj" +
        "CZqcMt4uQSJgqkYxIc1HfIgdSnVUMt/ARWndwLrrdsCtozkIgFyX5UgujSMtDXAUkqNZB5" +
        "OXPWi7xhzYdtUBUFTKnoSkcxiwXM5flC1xJg+Do/o6k2GqWGNiymBIMJ9lLFsH0fiEGQmM" +
        "VlaJYQshPJFkm9Kr6wSKfC/S1eVtA3TVhR+wIDAQAB",

        "OU=TC TrustCenter Class 1 CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCwKeu0drOu17ZbtF7nveOxnEkEV1uhq9" +
        "l/Exv9umGr2Odx3y0AlF1RSH0j73VihJA8Ch9ZEXQvjoCl/TACPSlSzXIaSSGcvMtSjkih" +
        "Y5bIEIUwaVd0RcBahsbVPeBoV30xaiSNRZc+MX5oZjJuJG3sMjbJQcrwMUTIo2HKG6A2Hw" +
        "IDAQAB",

        "OU=TC TrustCenter Class 2 CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDaOOjtMgApcYMBDb+MAdzaxq05pKmKL9" +
        "WLXGhfUMZi9Wa9ypEi7KodUdc9s1Gyg05dy0mw8ExV5Wstx4ULMBySToLUygLt92++3ODj" +
        "FLgFU/Ka9FaLWp6Fk9G0glauTbuoS1cWvP74WJ74KY2we814yU+si2cM8Zz7/FebV1xPDQ" +
        "IDAQAB",

        "OU=TC TrustCenter Class 3 CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC2tME1BS4NjeygQGocDiemUJJrUBsH3i" +
        "7ndszg2vyEqF6MY2orTdlOAnYRwQvyjXnKALbxsA7X+6QXPa+raXqWJ7+vM6GaKlmqxLU3" +
        "CPISpTG2Q/UylnEoKKuNKIbfu+7jDH0w1sNSq49dJ5xrwKPnBWtXSUSzbupkz9KOelB3dw" +
        "IDAQAB",

        "OU=TC TrustCenter Class 4 CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC/L2PWNnuyDdNV9WRs5iVdxrTIFLolOI" +
        "PrVmKlVallo/QjmcJLudDNVGemo6CjqTMrduS9rXey7VwSdMPFtg9SmnKTQ5BiZhUPRaXd" +
        "4N24b0BuV8F5cqNgqrp2HRKJU1r8Ar7hCRPFSi/cPYsZrdeLJEX7TPTNXDUdKUxR8/JsVQ" +
        "IDAQAB",

        "CN=Thawte Premium Server CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDSNjZqi9fCW57agUFijzjuSQRV1tDvHB" +
        "uVFkfvGEg1OlL0K2oGjzsv6lbjr4aNnhf3nrRldQJN78sJoiFR2JvQZ9C6DZIGFHPUk8uX" +
        "KgCcXE4MvPoVUvzyRG7aEUpuCJ8vLeP5qjqGc7ZGU1jIiQW9gxG4cz+qB430Qk3nQJ0cNw" +
        "IDAQAB",

        "C=hk, O=C&W HKT SecureNet CA SGC Root",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqFNPj0Pdr+zBtA0bX7cIoprIQu" +
        "Nt1yUa3+DKvC8iJPlpIr0arVHncfe1dtTzPsg+EdBNe5keGLeezT5hG0URS1sm3Ck8AE0R" +
        "2h2Pnh903hVAvDDJD9/4LXzYjZ2g4J+wzydgzzgRCO82L3xONh0mAqf01FBDgUnr3beWFD" +
        "BjMtEDzSG8N5EePmWuFoL2FWBLUTuW5RnowvemBYE6qH8YWD53w1kAg/T1eUlgpy4DPgH9" +
        "heLfoZqJ2fhkCiuEzUPNJTUAXjBmdKHHCHWsSSeC17CVNW4dmYDrkqAtWtY4u7VHJ6sazL" +
        "9TU8FGsm/o101XEd2wNUgfqybqVg24CjC22wIDAQAB",

        "CN=Thawte Server CA",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDTpFBuyP9Wa+bPXbbqDGh1R6KqwtqEJf" +
        "yo9EdR2oW1IHSUhh4PdcnpCGH1Bm0wbhUZAulSwGLbTZme4moMRDjN/r7jZAlwxf6xaym2" +
        "L0nIO9QnBCUQly/nkG3AKEKZ10xD3sP1IW1Un13DWOHA5NlbsLjctHvfNjrCtWYiEtaHDQ" +
        "IDAQAB",

        "CN=UTN - DATACorp SGC",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3+5YEKIrblXEjr8uRgnn4AgPLi" +
        "t6E5Qbvfa2gI5lBZMAHryv4g+OGQ0SR+ysraP6LnD43m77VkIVni5c7yPeIbkFdicZD0/W" +
        "w5y0vpQZY/KmEQrrU0icvvIpOxboGqBMpsn0GFlowHDyUwDAXlCCpVZvNvlK4ESGoE1O1k" +
        "duSUrLZ9emxAW5jh70/P/N5zbgnAVssjMiFdC04MwXwLLA9P4yPykqlXvY8qdOD1R8oQ2A" +
        "swkDwf9c3V6aPryuvEeKaq5xyh+xKrhfQgUL7EYw0XILyulWbfXv33i+Ybqypa4ETLyorG" +
        "kVl73v67SMvzX41MPRKA5cOp9wGDMgd8SirwIDAQAB",

        "CN=UTN-USERFirst-Hardware",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsffDOD+0qH/POYJRZ9Btn9L/WP" +
        "PnnyvsDYlUmbk4mRb34CF5SMK7YXQSlh08anLVPBBnOjntKxPNZuuVCTOkbJex6MbswXV5" +
        "nEZejavQav25KlUXEFSzGfCa9vGxXbanbfvgcRdrooj7AN/+GjF3DJoBerEy4ysBBzhuw6" +
        "VeI7xFm3tQwckwj9vlK3rTW/szQB6g1ZgXvIuHw4nTXaCOsqqq9o5piAbF+okh8widaS4J" +
        "M5spDUYPjMxJNLBpUb35Bs1orWZMvD6sYb0KiA7I3z3ufARMnQpea5HW7sftKI2rTYeJc9" +
        "BupNAeFosU4XZEA39jrOTNSZzFkvSrMqFIWwIDAQAB",

        "CN=UTN-USERFirst-Network Applications",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAs/uRoeQ2VYWsBjRboJpYsvi1Dw" +
        "V3g64ysXaSaOwjSsl2P+Octjd5A7mraY0HJbYZZ+SwGxhzYUrofs3TL2TjpnwM+heAow1H" +
        "iU9RcS/u/D/5uBaAh4mTJSCaQ4JpJHYoWTWhHcB/gwZkFiAs00mkhbTAYX9RCPhoFZGAy6" +
        "XV7js69IQEXmBZp4w0cu64eMXROxJKb35lJ7mkVcW5b0OkxR0smcBSpHhMFbNAmAhrQ8YB" +
        "sHp79WscIj/L7/+o0DpLdhWe0tHGLuPbVxsyorhv6IamP3Cr5XCSq0QeQFD7nKNi5GxuoM" +
        "je4oBC+ukv6M4yBI98jbccozU8Fd2ew66XpQIDAQAB",

        "CN=VeriSign Class 3 Public Primary Certification Authority - G3",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy7qcUvx4Hxoebxs3c734yWuUEj" +
        "BP8DZH9dCRCvUXyKVhwRZATfuKYZDldiDBEQZ9qyxupvURQY76La0qYVmkZyZM0Oi8Ultw" +
        "IARY0XrJpGm8gxdkrQWLvNBYzo2M9evwQkkLnZcnZzJu4a6TFRxwvCBNLxjekojobIVXER" +
        "rpfuMmEVSiRZZVg8owiejc2KPtKoA/f3llVz4VIGYIL5WTv6pHL6hGl/AS4v7CCitR5nbm" +
        "t0a34g2mzKjDTFlVieboU1wc6p3wYhYLp8lfDPDewnbOr/dq8vpBpqIzFMnlemPTnmI31Y" +
        "Vlng7mUyR0G14dElNbxyzng0k7Fa6KaLlXlwIDAQAB",

        "CN=VeriSign Class 4 Public Primary Certification Authority - G3",
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArculEWnGWavxj7UZD1bOzLUfIO" +
        "SeJiVL4HNliVne0IPk9Q+1u63xfOgh/OToDO58RSIZdpK0E7cgWwn6Ya6o8qWNhcIq1t5m" +
        "NtKbAvSokmB8nGm0jyQe0IZS9jKcQVgeIr3NRWKVCG7QZt1ToszwENxUc4sEoUYzM1wXQL" +
        "meTdPzvlWD6LGJjlp8mpYikDuIJfLSU4gCDAt48uY3F0swRgfkgG2m2JYu6Cz4EbM4DWam" +
        "m+rJI1vbjuLzE44aWS2qAvDspIdm3ME/9di59OyCxtI9lR3lwE+EydmjRCgGatdFrPBrau" +
        "9OX/gRgh44YzRmUNQ+k3P6MMNmrf+TLZfvAwIDAQAB",

        "OU=VeriSign Trust Network",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC68OTP+cSuhVS5B1f5j8V/aBH4xBewRN" +
        "zjMHPVKmIquNDMHO0oW369atyzkSTKQWI8/AIBvxwWMZQFl3Zuoq29YRdsTjCG8FE3KlDH" +
        "qGKB3FtKqsGgtG7rL+VXxbErQHDbWk2hjh+9Ax/YA9SPTJlxvOKCzFjomDqG04Y48wApHw" +
        "IDAQAB",

        "OU=VeriSign Trust Network",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC68OTP+cSuhVS5B1f5j8V/aBH4xBewRN" +
        "zjMHPVKmIquNDMHO0oW369atyzkSTKQWI8/AIBvxwWMZQFl3Zuoq29YRdsTjCG8FE3KlDH" +
        "qGKB3FtKqsGgtG7rL+VXxbErQHDbWk2hjh+9Ax/YA9SPTJlxvOKCzFjomDqG04Y48wApHw" +
        "IDAQAB",

        "OU=VeriSign Trust Network",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDMXtERXVxp0KvTuWpMmR9ZmDCOFoUgRm" +
        "1HP9SFIIThbbP4pO0M8RcPO/mn+SXXwc+EY/J8Y8+iR/LGWzOOZEAEaMGAuWQcRXfH2G71" +
        "lSk8UOg013gfqLptQ5GVj0VXXn7F+8qkBOvqlzdUMG+7AUcyM83cV5tkaWH4mx0ciU9cZw" +
        "IDAQAB",

        "OU=VeriSign Trust Network",
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDMXtERXVxp0KvTuWpMmR9ZmDCOFoUgRm" +
        "1HP9SFIIThbbP4pO0M8RcPO/mn+SXXwc+EY/J8Y8+iR/LGWzOOZEAEaMGAuWQcRXfH2G71" +
        "lSk8UOg013gfqLptQ5GVj0VXXn7F+8qkBOvqlzdUMG+7AUcyM83cV5tkaWH4mx0ciU9cZw" +
        "IDAQAB"
    };

    public static boolean alwaysFalse = false;

    static class entropySpinner extends Thread {
        volatile boolean stop = false;
        byte counter = 0;
        entropySpinner() { start(); }
        public void run() {
            while (true) {
                counter++;

                // without this line, GCJ will over-optimize this loop into an infinite loop. Argh.
                if (alwaysFalse) stop = true;

                if (stop) return;
            }
        }
    }
    
    static { 

        entropySpinner[] spinners = new entropySpinner[10];
        for(int i=0; i<spinners.length; i++) spinners[i] = new entropySpinner();

        for(int i=0; i<pad1.length; i++) pad1[i] = (byte)0x36;
        for(int i=0; i<pad2.length; i++) pad2[i] = (byte)0x5C;
        for(int i=0; i<pad1_sha.length; i++) pad1_sha[i] = (byte)0x36;
        for(int i=0; i<pad2_sha.length; i++) pad2_sha[i] = (byte)0x5C;

        try { 
            //if (Log.on) Log.log(TinySSL.class, "reading in trusted root public keys..."); 
            trusted_CA_public_keys = new SubjectPublicKeyInfo[base64_encoded_trusted_CA_public_keys.length / 2];
            trusted_CA_public_key_identifiers = new String[base64_encoded_trusted_CA_public_keys.length / 2];
            for(int i=0; i<base64_encoded_trusted_CA_public_keys.length; i+=2) {
                trusted_CA_public_key_identifiers[i/2] = base64_encoded_trusted_CA_public_keys[i];
                byte[] b = Base64.decode(base64_encoded_trusted_CA_public_keys[i+1]);
                DERInputStream dIn = new DERInputStream(new ByteArrayInputStream(b)); 
                trusted_CA_public_keys[i/2] = new SubjectPublicKeyInfo((DERConstructedSequence)dIn.readObject());
            }

        } catch (Exception e) { 
            //if (Log.on) Log.log(TinySSL.class, e);
        } 
        
        //if (Log.on) Log.log(TinySSL.class, "generating entropy..."); 
        randpool = new byte[10];
        try { Thread.sleep(100); } catch (Exception e) { }
        for(int i=0; i<spinners.length; i++) {
            spinners[i].stop = true;
            randpool[i] = spinners[i].counter;
        }
        
        MD5Digest md5 = new MD5Digest();
        md5.update(randpool, 0, randpool.length);
        intToBytes(System.currentTimeMillis(), randpool, 0, 4); md5.update(randpool, 0, 4);
        intToBytes(Runtime.getRuntime().freeMemory(), randpool, 0, 4); md5.update(randpool, 0, 4);
        intToBytes(Runtime.getRuntime().totalMemory(), randpool, 0, 4); md5.update(randpool, 0, 4);
        intToBytes(System.identityHashCode(TinySSL.class), randpool, 0, 4); md5.update(randpool, 0, 4);
/*
        Properties p = System.getProperties();
        for(Enumeration e = p.propertyNames(); e.hasMoreElements();) {
            String s = (String)e.nextElement();
            byte[] b = s.getBytes();
            md5.update(b, 0, b.length);
            b = p.getProperty(s).getBytes();
            md5.update(b, 0, b.length);
        }
*/
        randpool = new byte[md5.getDigestSize()];
        md5.doFinal(randpool, 0);

        //if (Log.on) Log.log(TinySSL.class, "TinySSL is initialized."); 
    } 


    /**
     *  A PKCS1 encoder which uses TinySSL's built-in PRNG instead of java.security.SecureRandom.
     *  This code was derived from BouncyCastle's org.bouncycastle.crypto.encoding.PKCS1Encoding.
     */
    private static class PKCS1 implements AsymmetricBlockCipher {
        private static int HEADER_LENGTH = 10;
        private AsymmetricBlockCipher engine;
        private boolean forEncryption;
        private boolean forPrivateKey;
        
        public PKCS1(AsymmetricBlockCipher cipher) { this.engine = cipher; }   
        public AsymmetricBlockCipher getUnderlyingCipher() { return engine; }

        public void init(boolean forEncryption, CipherParameters param) {
            engine.init(forEncryption, (AsymmetricKeyParameter)param);
            this.forPrivateKey = ((AsymmetricKeyParameter)param).isPrivate();
            this.forEncryption = forEncryption;
        }

        public int getInputBlockSize() { return engine.getInputBlockSize() - (forEncryption ? HEADER_LENGTH : 0); }
        public int getOutputBlockSize() { return engine.getOutputBlockSize() - (forEncryption ? 0 : HEADER_LENGTH); }

        public byte[] processBlock(byte[] in, int inOff, int inLen) throws InvalidCipherTextException {
            return forEncryption ? encodeBlock(in, inOff, inLen) : decodeBlock(in, inOff, inLen);
        }

        private byte[] encodeBlock(byte[] in, int inOff, int inLen) throws InvalidCipherTextException {
            byte[]  block = new byte[engine.getInputBlockSize()];
            if (forPrivateKey) {
                block[0] = 0x01;                        // type code 1
                for (int i = 1; i != block.length - inLen - 1; i++)
                    block[i] = (byte)0xFF;
            } else {
                getRandomBytes(block, 0, block.length);
                block[0] = 0x02;                        // type code 2

                // a zero byte marks the end of the padding, so all
                // the pad bytes must be non-zero.
                for (int i = 1; i != block.length - inLen - 1; i++)
                    while (block[i] == 0)
                        getRandomBytes(block, i, 1);
            }

            block[block.length - inLen - 1] = 0x00;       // mark the end of the padding
            System.arraycopy(in, inOff, block, block.length - inLen, inLen);
            return engine.processBlock(block, 0, block.length);
        }

        private byte[] decodeBlock(byte[] in, int inOff, int inLen) throws InvalidCipherTextException {
            byte[]  block = engine.processBlock(in, inOff, inLen);
            if (block.length < getOutputBlockSize())
                throw new InvalidCipherTextException("block truncated");
            if (block[0] != 1 && block[0] != 2)
                throw new InvalidCipherTextException("unknown block type");

            // find and extract the message block.
            int start;
            for (start = 1; start != block.length; start++)
                if (block[start] == 0)
                    break;
            start++;           // data should start at the next byte

            if (start >= block.length || start < HEADER_LENGTH)
                throw new InvalidCipherTextException("no data in block");

            byte[]  result = new byte[block.length - start];
            System.arraycopy(block, start, result, 0, result.length);
            return result;
        }
    }

}

