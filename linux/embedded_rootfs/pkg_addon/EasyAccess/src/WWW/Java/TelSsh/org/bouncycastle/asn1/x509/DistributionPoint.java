package org.bouncycastle.asn1.x509;

import org.bouncycastle.asn1.*;

public class DistributionPoint
    implements DEREncodable
{
    ASN1Sequence  seq = null;

    public static DistributionPoint getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }

    public static DistributionPoint getInstance(
        Object obj)
    {
        if(obj == null || obj instanceof DistributionPoint) 
        {
            return (DistributionPoint)obj;
        }
        
        if(obj instanceof ASN1Sequence) 
        {
            return new DistributionPoint((ASN1Sequence)obj);
        }
        
        throw new IllegalArgumentException("Invalid DistributionPoint: " + obj.getClass().getName());
    }

    public DistributionPoint(
        ASN1Sequence seq)
    {
        this.seq = seq;
    }
    
    public DistributionPoint(
        DistributionPointName   distributionPoint,
        ReasonFlags             reasons,
        GeneralNames            cRLIssuer)
    {
        DEREncodableVector  v = new DEREncodableVector();

        if (distributionPoint != null)
        {
            v.add(new DERTaggedObject(0, distributionPoint));
        }

        if (reasons != null)
        {
            v.add(new DERTaggedObject(1, reasons));
        }

        if (cRLIssuer != null)
        {
            v.add(new DERTaggedObject(2, cRLIssuer));
        }

        seq = new DERSequence(v);
    }

    /**
     * <pre>
     * DistributionPoint ::= SEQUENCE {
     *      distributionPoint [0] DistributionPointName OPTIONAL,
     *      reasons           [1] ReasonFlags OPTIONAL,
     *      cRLIssuer         [2] GeneralNames OPTIONAL
     * }
     * </pre>
     */
    public DERObject getDERObject()
    {
        return seq;
    }
}
