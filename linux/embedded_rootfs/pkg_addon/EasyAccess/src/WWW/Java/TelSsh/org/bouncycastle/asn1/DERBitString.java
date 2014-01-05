package org.bouncycastle.asn1;

import java.io.*;

public class DERBitString
    extends DERObject
{
    protected byte[]      data;
    protected int         padBits;

    /**
     * return the correct number of pad bits for a bit string defined in
     * a 16 bit constant
     */
    static protected int getPadBits(
        int bitString)
    {
        int val;

        if (bitString == 0)
        {
            return 7;
        }

        if (bitString > 255)
        {
            val = ((bitString >> 8) & 0xFF);
        }
        else
        {
            val = (bitString & 0xFF);
        }

        int bits = 1;

        while (((val <<= 1) & 0xFF) != 0)
        {
            bits++;
        }

        return 8 - bits;
    }

    /**
     * return the correct number of bytes for a bit string defined in
     * a 16 bit constant
     */
    static protected byte[] getBytes(
        int bitString)
    {
        if (bitString > 255)
        {
            byte[]  bytes = new byte[2];

            bytes[0] = (byte)(bitString & 0xFF);
            bytes[1] = (byte)((bitString >> 8) & 0xFF);

            return bytes;
        }
        else
        {
            byte[]  bytes = new byte[1];

            bytes[0] = (byte)(bitString & 0xFF);

            return bytes;
        }
    }

    /**
     * return a Bit String from the passed in object
     *
     * @exception IllegalArgumentException if the object cannot be converted.
     */
    public static DERBitString getInstance(
        Object  obj)
    {
        if (obj == null || obj instanceof DERBitString)
        {
            return (DERBitString)obj;
        }

        if (obj instanceof ASN1OctetString)
        {
            byte[]  bytes = ((ASN1OctetString)obj).getOctets();
            int     padBits = bytes[0];
            byte[]  data = new byte[bytes.length - 1];

            System.arraycopy(bytes, 1, data, 0, bytes.length - 1);

            return new DERBitString(data, padBits);
        }

        if (obj instanceof ASN1TaggedObject)
        {
            return getInstance(((ASN1TaggedObject)obj).getObject());
        }

        throw new IllegalArgumentException("illegal object in getInstance: " + obj.getClass().getName());
    }

    /**
     * return a Bit String from a tagged object.
     *
     * @param obj the tagged object holding the object we want
     * @param explicit true if the object is meant to be explicitly
     *              tagged false otherwise.
     * @exception IllegalArgumentException if the tagged object cannot
     *               be converted.
     */
    public static DERBitString getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(obj.getObject());
    }
    
    protected DERBitString(
        byte    data,
        int     padBits)
    {
        this.data = new byte[1];
        this.data[0] = data;
        this.padBits = padBits;
    }

    /**
     * @param data the octets making up the bit string.
     * @param padBits the number of extra bits at the end of the string.
     */
    public DERBitString(
        byte[]  data,
        int     padBits)
    {
        this.data = data;
        this.padBits = padBits;
    }

    public DERBitString(
        byte[]  data)
    {
        this(data, 0);
    }

    public DERBitString(
        DEREncodable  obj)
    {
        try
        {
            ByteArrayOutputStream   bOut = new ByteArrayOutputStream();
            DEROutputStream         dOut = new DEROutputStream(bOut);

            dOut.writeObject(obj);
            dOut.close();

            this.data = bOut.toByteArray();
            this.padBits = 0;
        }
        catch (IOException e)
        {
            throw new IllegalArgumentException("Error processing object : " + e.toString());
        }
    }

    public byte[] getBytes()
    {
        return data;
    }

    public int getPadBits()
    {
        return padBits;
    }

    void encode(
        DEROutputStream  out)
        throws IOException
    {
        byte[]  bytes = new byte[getBytes().length + 1];

        bytes[0] = (byte)getPadBits();
        System.arraycopy(getBytes(), 0, bytes, 1, bytes.length - 1);

        out.writeEncoded(BIT_STRING, bytes);
    }
    
    public boolean equals(
        Object  o)
    {
        if (o == null || !(o instanceof DERBitString))
        {
            return false;
        }

        DERBitString  other = (DERBitString)o;

        if (data.length != other.data.length)
        {
            return false;
        }

        for (int i = 0; i != data.length; i++)
        {
            if (data[i] != other.data[i])
            {
                return false;
            }
        }

        return (padBits == other.padBits);
    }
}
