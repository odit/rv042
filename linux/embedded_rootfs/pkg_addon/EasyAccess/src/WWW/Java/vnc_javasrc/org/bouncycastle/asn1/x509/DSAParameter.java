package org.bouncycastle.asn1.x509;

import java.math.BigInteger;
import java.util.*;

import org.bouncycastle.asn1.*;

public class DSAParameter
    implements DEREncodable
{
    DERInteger      p, q, g;

    public static DSAParameter getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }

    public static DSAParameter getInstance(
        Object obj)
    {
        if(obj == null || obj instanceof DSAParameter) 
        {
            return (DSAParameter)obj;
        }
        
        if(obj instanceof ASN1Sequence) 
        {
            return new DSAParameter((ASN1Sequence)obj);
        }
        
        throw new IllegalArgumentException("Invalid DSAParameter: " + obj.getClass().getName());
    }

    public DSAParameter(
        BigInteger  p,
        BigInteger  q,
        BigInteger  g)
    {
        this.p = new DERInteger(p);
        this.q = new DERInteger(q);
        this.g = new DERInteger(g);
    }

    public DSAParameter(
        ASN1Sequence  seq)
    {
        Enumeration     e = seq.getObjects();

        p = (DERInteger)e.nextElement();
        q = (DERInteger)e.nextElement();
        g = (DERInteger)e.nextElement();
    }

    public BigInteger getP()
    {
        return p.getPositiveValue();
    }

    public BigInteger getQ()
    {
        return q.getPositiveValue();
    }

    public BigInteger getG()
    {
        return g.getPositiveValue();
    }

    public DERObject getDERObject()
    {
        DEREncodableVector  v = new DEREncodableVector();

        v.add(p);
        v.add(q);
        v.add(g);

        return new DERSequence(v);
    }
}
