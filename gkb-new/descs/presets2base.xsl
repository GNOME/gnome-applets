<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  >
<xsl:output method="xml"
            encoding="UTF-8"
            doctype-system="xkb.dtd"
            indent="yes"/>
  
  <!-- Transform all "simple" elements as they are -->
  <xsl:template match="keymaps">
<xkbConfigRegistry>
  <modelList>
    <model>
      <configItem>
        <name>generic</name>
        <_description>Generic Keyboard</_description>
      </configItem>
    </model>
  </modelList>
  <layoutList>
    <xsl:apply-templates select="keymap[substring(command,1,9)='gkb_xmmap']"/>
  </layoutList>
  <optionList>
  </optionList>
</xkbConfigRegistry>
  </xsl:template>

  <xsl:template match="keymap">
    <layout>
      <configItem>
        <name><xsl:value-of select="substring(command,11)"/></name>
        <_shortDescription><xsl:value-of select="_name"/></_shortDescription>
        <_description><xsl:value-of select="_name"/></_description>
      </configItem>
    </layout>
  </xsl:template>

</xsl:stylesheet>
