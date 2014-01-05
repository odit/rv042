package org.bouncycastle.asn1;

import java.io.*;
import java.util.*;

public abstract class ASN1Sequence
    extends DERObject
{
    private Vector seq = new Vector();

    /**
     * return an ASN1Sequence from the given object.
     *
     * @param obj the object we want converted.
     * @exception IllegalArgumentException if the object cannot be converted.
     */
    public static ASN1Sequence getInstance(
        Object  obj)
    {
        if (obj == null || obj instanceof ASN1Sequence)
        {
            return (ASN1Sequence)obj;
        }

        throw new IllegalArgumentException("unknown object in getInstance");
    }

    /**
     * Return an ASN1 sequence from a tagged object. There is a special
     * case here, if an object appears to have been explicitly tagged on 
     * reading but we were expecting it to be implictly tagged in the 
     * normal course of events it indicates that we lost the surrounding
     * sequence - so we need to add it back (this will happen if the tagged
     * object is a sequence that contains other sequences). If you are
     * dealing with implicitly tagged sequences you really <b>should</b>
     * be using this method.
     *
     * @param obj the tagged object.
     * @param explicit true if the object is meant to be explicitly tagged,
     *          false otherwise.
     * @exception IllegalArgumentException if the tagged object cannot
     *          be converted.
     */
    public static ASN1Sequence getInstance(
        ASN1TaggedObject    obj,
        boolean             explicit)
    {
        if (explicit)
        {
            if (!obj.isExplicit())
            {
                throw new IllegalArgumentException("object implicit - explicit expected.");
            }

            return (ASN1Sequence)obj.getObject();
        }
        else
        {
            //
            // constructed object which appears to be explicitly tagged
            // when it should be implicit means we have to add the
            // surrounding sequence.
            //
            if (obj.isExplicit())
            {
                ASN1Sequence    seq;

                if (obj instanceof BERTaggedObject)
                {
                    seq = new BERConstructedSequence();
                }
                else
                {
                    seq = new DERConstructedSequence();
                }

                seq.addObject(obj.getObject());

                return seq;
            }
            else
            {
                ASN1Sequence    seq;

                if (obj.getObject() instanceof ASN1Sequence)
                {
                    return (ASN1Sequence)obj.getObject();
                }
            }
        }

        throw new IllegalArgumentException(
                "unknown object in getInstanceFromTagged");
    }

    public Enumeration getObjects()
    {
        return seq.elements();
    }

    /**
     * return the object at the sequence postion indicated by index.
     *
     * @param the sequence number (starting at zero) of the object
     * @return the object at the sequence postion indicated by index.
     */
    public DEREncodable getObjectAt(
        int index)
    {
        return (DEREncodable)seq.elementAt(index);
    }

    /**
     * return the number of objects in this sequence.
     *
     * @return the number of objects in this sequence.
     */
    public int size()
    {
        return seq.size();
    }

    public int hashCode()
    {
        Enumeration             e = this.getObjects();
        int                     hashCode = 0;

        while (e.hasMoreElements())
        {
            hashCode ^= e.nextElement().hashCode();
        }

        return hashCode;
    }

    public boolean equals(
        Object  o)
    {
        if (o == null || !(o instanceof ASN1Sequence))
        {
            return false;
        }

        ASN1Sequence   other = (ASN1Sequence)o;

        if (this.size() != other.size())
        {
            return false;
        }

        Enumeration s1 = this.getObjects();
        Enumeration s2 = other.getObjects();

        while (s1.hasMoreElements())
        {
            if (!s1.nextElement().equals(s2.nextElement()))
            {
                return false;
            }
        }

        return true;
    }

    protected void addObject(
        DEREncodable obj)
    {
        seq.addElement(obj);
    }

    abstract void encode(DEROutputStream out)
        throws IOException;
}
