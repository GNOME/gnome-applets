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
    <group allowMultipleSelection="false">
      <configItem>
        <name>grp</name>
        <_description>Layout shift behaviour</_description>
      </configItem>
      <option>
        <configItem>
          <name>grp:ralt_toggle</name>
          <_description>Right Alt key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:lalt_toggle</name>
          <_description>Left Alt key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:caps_toggle</name>
          <_description>CapsLock key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:shift_caps_toggle</name>
          <_description>Shift+CapsLock changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:shifts_toggle</name>
          <_description>Both Shift keys together change layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:alts_toggle</name>
          <_description>Both Alt keys together change layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:ctrls_toggle</name>
          <_description>Both Ctrl keys together change layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:ctrl_shift_toggle</name>
          <_description>Control+Shift changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:ctrl_alt_toggle</name>
          <_description>Alt+Control changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:alt_shift_toggle</name>
          <_description>Alt+Shift changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:menu_toggle</name>
          <_description>Menu key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:lwin_toggle</name>
          <_description>Left Win-key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:rwin_toggle</name>
          <_description>Right Win-key changes layout.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:lshift_toggle</name>
          <_description>Left Shift key changes group.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:rshift_toggle</name>
          <_description>Right Shift key changes group.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:lctrl_toggle</name>
          <_description>Left Ctrl key changes group.</_description>
        </configItem>
      </option>
      <option>
        <configItem>
          <name>grp:rctrl_toggle</name>
          <_description>Right Ctrl key changes group.</_description>
        </configItem>
      </option>
    </group>
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
