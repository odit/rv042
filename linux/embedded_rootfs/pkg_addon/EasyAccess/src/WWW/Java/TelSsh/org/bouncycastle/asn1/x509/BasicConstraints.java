package org.bouncycastle.asn1.x509;

import java.math.BigInteger;

import org.bouncycastle.asn1.*;

public class BasicConstraints
    implements DEREncodable
{
    DERBoolean  cA = new DERBoolean(false);
    DERInteger  pathLenConstraint = null;

    public static BasicConstraints getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }

    public static BasicConstraints getInstance(
        Object  obj)
    {
        if (obj instanceof BasicConstraints)
        {
            return (BasicConstraints)obj;
        }
        else if (obj instanceof ASN1Sequence)
        {
            return new BasicConstraints((ASN1Sequence)obj);
        }

        throw new IllegalArgumentException("unknown object in factory");
    }
	
    public BasicConstraints(
        ASN1Sequence   seq)
    {
        if (seq.size() != 0)
        {
            this.cA = (DERBoolean)seq.getObjectAt(0);
            this.pathLenConstraint = (DERInteger)seq.getObjectAt(1);
        }
    }

    public BasicConstraints(
        boolean cA,
        int     pathLenConstraint)
    {
        this.cA = new DERBoolean(cA);
        this.pathLenConstraint = new DERInteger(pathLenConstraint);
    }

    public BasicConstraints(
        boolean cA)
    {
        this.cA = new DERBoolean(cA);
        this.pathLenConstraint = null;
    }

    public boolean isCA()
    {
        return cA.isTrue();
    }

    public BigInteger getPathLenConstraint()
    {
        if (pathLenConstraint != null)
        {
            return pathLenConstraint.getValue();
        }

        return null;
    }

    /**
     * <pre>
     * BasicConstraints := SEQUENCE {
     *    cA                  BOOLEAN DEFAULT FALSE,
     *    pathLenConstraint   INTEGER (0..MAX) OPTIONAL
     * }
     * </pre>
     */
    public DERObject getDERObject()
    {
        DERConstructedSequence  seq = new DERConstructedSequence();

        seq.addObject(cA);

        if (pathLenConstraint != null)
        {
            seq.addObject(pathLenConstraint);
        }

        return seq;
    }

    public String toString()
    {
        return "BasicConstraints: isCa(" + this.isCA() + "), pathLenConstraint = " + pathLenConstraint.getValue();
    }
}
