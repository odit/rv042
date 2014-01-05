// Decompiled by Jad v1.5.7f. Copyright 2000 Pavel Kouznetsov.
// Jad home page: http://www.geocities.com/SiliconValley/Bridge/8617/jad.html
// Decompiler options: packimports(3) 
// Source File Name:   SignedAttributes.java

package org.bouncycastle.asn1.cms;

import java.util.Vector;
import org.bouncycastle.asn1.*;

// Referenced classes of package org.bouncycastle.asn1.cms:
//            Attribute

public class SignedAttributes
    implements DEREncodable
{

    public SignedAttributes(Vector vector)
    {
        setAttributes(vector);
    }

    public SignedAttributes(DERConstructedSet derconstructedset)
    {
        attributes = derconstructedset;
    }

    public SignedAttributes(SignedAttributes signedattributes)
    {
        attributes = signedattributes.attributes;
    }

    public static SignedAttributes getInstance(Object obj)
    {
        if(obj == null)
            return null;
        if(obj instanceof SignedAttributes)
            return (SignedAttributes)obj;
        if(obj instanceof DERConstructedSet)
            return new SignedAttributes((DERConstructedSet)obj);
        if(obj instanceof DERTaggedObject)
            return getInstance(((DERTaggedObject)obj).getObject());
        else
            throw new IllegalArgumentException("Invalid SignedAttributes");
    }

    public static SignedAttributes newInstance(Object obj)
    {
        if(obj == null)
            return null;
        if(obj instanceof SignedAttributes)
            return new SignedAttributes((SignedAttributes)obj);
        if(obj instanceof DERConstructedSet)
            return new SignedAttributes((DERConstructedSet)obj);
        if(obj instanceof DERTaggedObject)
            return getInstance(((DERTaggedObject)obj).getObject());
        else
            throw new IllegalArgumentException("Invalid SignedAttributes");
    }

    public Vector getAttributes()
    {
        int i = attributes.getSize();
        Vector vector = new Vector();
        for(int j = 0; j < i; j++)
            vector.addElement(Attribute.getInstance(attributes.getObjectAt(j)));

        return vector;
    }

    private void setAttributes(Vector vector)
    {
        int i = vector.size();
        attributes = new DERConstructedSet();
        for(int j = 0; j < i; j++)
            attributes.addObject(Attribute.getInstance(vector.elementAt(j)));

    }

    public DERObject getDERObject()
    {
        return attributes;
    }

    private DERConstructedSet attributes;
}
