package org.bouncycastle.asn1.x509;

import java.io.*;
import java.util.Enumeration;

import org.bouncycastle.asn1.*;

public class AlgorithmIdentifier
    implements DEREncodable
{
    private DERObjectIdentifier objectId;
    private DEREncodable        parameters;
    private boolean             parametersDefined = false;
	
    public static AlgorithmIdentifier getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }
    
    public static AlgorithmIdentifier getInstance(
        Object  obj)
    {
        if (obj instanceof AlgorithmIdentifier)
        {
            return (AlgorithmIdentifier)obj;
        }
        
        if (obj instanceof DERObjectIdentifier)
        {
            return new AlgorithmIdentifier((DERObjectIdentifier)obj);
        }

        if (obj instanceof String)
        {
            return new AlgorithmIdentifier((String)obj);
        }

        if (obj instanceof ASN1Sequence)
        {
            return new AlgorithmIdentifier((ASN1Sequence)obj);
        }

        throw new IllegalArgumentException("unknown object in factory");
    }

    public AlgorithmIdentifier(
        DERObjectIdentifier     objectId)
    {
        this.objectId = objectId;
    }

    public AlgorithmIdentifier(
        String     objectId)
    {
        this.objectId = new DERObjectIdentifier(objectId);
    }

    public AlgorithmIdentifier(
        DERObjectIdentifier     objectId,
        DEREncodable            parameters)
    {
        parametersDefined = true;
        this.objectId = objectId;
        this.parameters = parameters;
    }

    public AlgorithmIdentifier(
        ASN1Sequence   seq)
    {
        objectId = (DERObjectIdentifier)seq.getObjectAt(0);

        if (seq.size() == 2)
        {
            parametersDefined = true;
            parameters = seq.getObjectAt(1);
        }
        else
        {
            parameters = null;
        }
    }

    public DERObjectIdentifier getObjectId()
    {
        return objectId;
    }

    public DEREncodable getParameters()
    {
        return parameters;
    }

    /**
     * <pre>
     *      AlgorithmIdentifier ::= SEQUENCE {
     *                            algorithm OBJECT IDENTIFIER,
     *                            parameters ANY DEFINED BY algorithm OPTIONAL }
     * </pre>
     */
    public DERObject getDERObject()
    {
        DERConstructedSequence  seq = new DERConstructedSequence();

        seq.addObject(objectId);

        if (parametersDefined)
        {
            seq.addObject(parameters);
        }

        return seq;
    }

    public boolean equals(
        Object  o)
    {
        if ((o == null) || !(o instanceof AlgorithmIdentifier))
        {
            return false;
        }

        AlgorithmIdentifier other = (AlgorithmIdentifier)o;

        if (!this.getObjectId().equals(other.getObjectId()))
        {
            return false;
        }

        if (this.getParameters() == null && other.getParameters() == null)
        {
            return true;
        }

        if (this.getParameters() == null || other.getParameters() == null)
        {
            return false;
        }

        ByteArrayOutputStream   b1Out = new ByteArrayOutputStream();
        ByteArrayOutputStream   b2Out = new ByteArrayOutputStream();
        DEROutputStream         d1Out = new DEROutputStream(b1Out);
        DEROutputStream         d2Out = new DEROutputStream(b2Out);

        try
        {
            d1Out.writeObject(this.getParameters());
            d2Out.writeObject(other.getParameters());

            byte[]  b1 = b1Out.toByteArray();
            byte[]  b2 = b2Out.toByteArray();

            if (b1.length != b2.length)
            {
                return false;
            }

            for (int i = 0; i != b1.length; i++)
            {
                if (b1[i] != b2[i])
                {
                    return false;
                }
            }
        }
        catch (Exception e)
        {
            return false;
        }

        return true;
    }
}
