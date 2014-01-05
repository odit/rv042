package org.bouncycastle.asn1;

import java.util.Vector;

/**
 * a general class for building up a vector of DER encodable objects
 */
public class DEREncodableVector
{
    private Vector  v = new Vector();

    public void add(
        DEREncodable   obj)
    {
        v.addElement(obj);
    }

    public DEREncodable get(
        int i)
    {
        return (DEREncodable)v.elementAt(i);
    }

    public int size()
    {
        return v.size();
    }
}
