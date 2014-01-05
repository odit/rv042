package org.bouncycastle.asn1.x509;

import java.util.Enumeration;

import org.bouncycastle.asn1.*;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;

/**
 * <pre>
 * DigestInfo::=SEQUENCE{
 *          digestAlgorithm  AlgorithmIdentifier,
 *          digest OCTET STRING }
 * </pre>
 */
public class DigestInfo
    implements DEREncodable
{
    private byte[]                  digest;
    private AlgorithmIdentifier     algId;

    public static DigestInfo getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }

    public static DigestInfo getInstance(
        Object  obj)
    {
        if (obj instanceof DigestInfo)
        {
            return (DigestInfo)obj;
        }
        else if (obj instanceof ASN1Sequence)
        {
            return new DigestInfo((ASN1Sequence)obj);
        }

        throw new IllegalArgumentException("unknown object in factory");
    }

    public DigestInfo(
        AlgorithmIdentifier  algId,
        byte[]               digest)
    {
        this.digest = digest;
        this.algId = algId;
    }

    public DigestInfo(
        ASN1Sequence  obj)
    {
        Enumeration             e = obj.getObjects();

        algId = AlgorithmIdentifier.getInstance(e.nextElement());
        digest = ((ASN1OctetString)e.nextElement()).getOctets();
    }

    public AlgorithmIdentifier getAlgorithmId()
    {
        return algId;
    }

    public byte[] getDigest()
    {
        return digest;
    }

    public DERObject getDERObject()
    {
        DEREncodableVector  v = new DEREncodableVector();

        v.add(algId);
        v.add(new DEROctetString(digest));

        return new DERSequence(v);
    }
}
