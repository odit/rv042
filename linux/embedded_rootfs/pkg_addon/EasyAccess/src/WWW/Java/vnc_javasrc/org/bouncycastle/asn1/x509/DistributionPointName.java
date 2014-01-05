package org.bouncycastle.asn1.x509;

import org.bouncycastle.asn1.*;

public class DistributionPointName
    implements DEREncodable
{
    DEREncodable        name;
    int                 type;

    public static final int FULL_NAME = 0;
    public static final int NAME_RELATIVE_TO_CRL_ISSUER = 1;

    public DistributionPointName(
        int             type,
        DEREncodable    name)
    {
        this.type = type;
        this.name = name;
    }

    /**
     * <pre>
     * DistributionPointName ::= CHOICE {
     *     fullName                 [0] GeneralNames,
     *     nameRelativeToCRLIssuer  [1] RelativeDistinguishedName
     * }
     * </pre>
     */
    public DERObject getDERObject()
    {
        return new DERTaggedObject(false, type, name);
    }
}
