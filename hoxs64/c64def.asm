

/*        VIC  6567      */
        
#define VIC_S0_X        0
#define VIC_S0_Y        1
#define VIC_S1_X        2
#define VIC_S1_Y        3
#define VIC_S2_X        4
#define VIC_S2_Y        5
#define VIC_S3_X        6
#define VIC_S3_Y        7
#define VIC_S4_X        8
#define VIC_S4_Y        9
#define VIC_S5_X        10
#define VIC_S5_Y        11
#define VIC_S6_X        12
#define VIC_S6_Y        13
#define VIC_S7_X        14
#define VIC_S7_Y        15
#define VIC_SP_MSB      16
#define VIC_SR1         17
#define VIC_IRQ_RASTER  18
#define VIC_LP_X        19
#define VIC_LP_Y        20
#define VIC_SP_EN       21
#define VIC_SR2         22
#define VIC_SP_EXPY     23
#define VIC_BASEADR     24
#define VIC_IRR         25
#define VIC_IMR         26
#define VIC_SP_PRIOR    27
#define VIC_SP_MCOLOR   28
#define VIC_SP_EXPX     29
#define VIC_SP_SCOLL    30
#define VIC_SP_BCOLL    31
#define VIC_EXTCOL      32
#define VIC_BCKCOL0     33
#define VIC_BCKCOL1     34
#define VIC_BCKCOL2     35
#define VIC_BCKCOL3     36
#define VIC_SP_MCOL0    37
#define VIC_SP_MCOL1    38
#define VIC_S0_COL      39
#define VIC_S1_COL      40
#define VIC_S2_COL      41
#define VIC_S3_COL      42
#define VIC_S4_COL      43
#define VIC_S5_COL      44
#define VIC_S6_COL      45
#define VIC_S7_COL      46

#define COL_SCHWARZ     0
#define COL_WEISS       1
#define COL_ROT         2
#define COL_TUERKIS     3
#define COL_VIOLETT     4
#define COL_GRUEN       5
#define COL_BLAU        6
#define COL_GELB        7
#define COL_ORANGE      8
#define COL_BRAUN       9
#define COL_HELLROT     10
#define COL_GRAU1       11
#define COL_GRAU2       12
#define COL_HELLGRUEN   13
#define COL_HELLBLAU    14
#define COL_GRAU3       15

#define   VIC           $d000


/*        SID 6581       */

#define SID_0FREQL      0
#define SID_0FREQH      1
#define SID_0PULSL      2
#define SID_0PULSH      3
#define SID_0SR         4
#define SID_0ATTDEC     5
#define SID_0SUSREL     6
#define SID_1FREQL      7
#define SID_1FREQH      8
#define SID_1PULSL      9
#define SID_1PULSH      10
#define SID_1SR         11
#define SID_1ATTDEC     12
#define SID_1SUSREL     13
#define SID_2FREQL      14
#define SID_2FREQH      15
#define SID_2PULSL      16
#define SID_2PULSH      17
#define SID_2SR         18
#define SID_2ATTDEC     19
#define SID_2SUSREL     20
#define SID_FILTL       21
#define SID_FILTH       22
#define SID_SR1         23
#define SID_SR2         24

#define   SID            $d400


/*        CIA  6526           */

#define CIA_DRA         0
#define CIA_DRB         1
#define CIA_DDRA        2
#define CIA_DDRB        3
#define CIA_TAL         4
#define CIA_TAH         5
#define CIA_TBL         6
#define CIA_TBH         7
#define CIA_TOD_THS     8
#define CIA_TOD_SEC     9
#define CIA_TOD_MIN     10
#define CIA_TOD_HR      11
#define CIA_SDR         12
#define CIA_ICR         13
#define CIA_CRA         14
#define CIA_CRB         15

#define CIA1            $dc00
#define CIA2            $dd00


/*        ACIA 6551                */

#define   ACIA_DR        0
#define   ACIA_SR        1
#define   ACIA_CMD       2
#define   ACIA_CTRL      3

#define   ACIA           $d600


/*        Basic                    */

#define   INT            $14
#define   PRGANF         $2b
#define   VARANF         $2d
#define   ARRANF         $2f
#define   ARREND         $31
#define   STRANF         $33
#define   STRPTR         $35
#define   RAMEND         $37

#define   VARNAME        $45
#define   VARADR         $47

#define   AKKU3          $57
#define   AKKU4          $5c

#define   FAC            $61
#define   ARG            $69


#define   CHRGET         $73
#define   CHRGOT         $79
#define   PRGPTR         $7a

#define   V_ERR          $0300
#define   V_WARM         $0302
#define   V_CONV2CODE    $0304
#define   V_CONV2ASC     $0306
#define   V_GETBEFADR    $0308
#define   V_GETAUSDR     $030a

#define   SYS_AKKU       $030c
#define   SYS_XR         $030d
#define   SYS_YR         $030e
#define   SYS_SR         $030f

#define   READY          $a474
#define   LINEIN         $a560
#define   INTOUT         $bdcd

/*        Betriebssystem           */

#define   STATUS         $90

#define   FNAMLEN        $b7
#define   LOGFNR         $b8
#define   SECADR         $b9
#define   DEVADR         $ba
#define   FNAMPTR        $bb

#define   IOANF          $c1
#define   IOEND          $c3

#define   LASTKEY        $c5
#define   NUMKEY         $c6
#define   REVFL          $c7
#define   INLINEEND      $c8
#define   INZEILE        $c9
#define   INSPALTE       $ca
#define   PRESSEDKEY     $cb
#define   CRSRFLASH      $cc
#define   CRSRFLASHCNT   $cd
#define   CHARUNDERCRSR  $ce
#define   CRSRFLASHFL    $cf
#define   KEYINPUTFL     $d0
#define   LINEADR        $d1
#define   CRSRSPALTE     $d3
#define   HKFL           $d4
#define   LENGTHOFLINE   $d5
#define   CRSRZEILE      $d6
#define   DIV            $d7
#define   NUMOFINS       $d8

#define   RS232INBUFPTR  $f7
#define   RS232OUTBUFPTR $f9

#define   P1             $fb       /* freier Pointer */
#define   P2             $fd       /* freier Pointer */

#define   INBUF          $200

#define   V_USR          $0311
#define   V_IRQ          $0314
#define   V_BRK          $0316
#define   V_NMI          $0318
#define   V_OPEN         $031a
#define   V_CLOSE        $031c
#define   V_CHKIN        $031e
#define   V_CKOUT        $0320
#define   V_CLRCH        $0322
#define   V_INPUT        $0324
#define   V_OUTPUT       $0326
#define   V_STOP         $0328
#define   V_GET          $032a
#define   V_CLALL        $032c
#define   V_WARMSTART    $032e
#define   V_LOAD         $0330
#define   V_SAVE         $0332

#define   SENDNAM        $f3d5
#define   CLSFIL         $f642

#define   INICIA         $ff84
#define   INIRAM         $ff87
#define   INIIO          $ff8a
#define   INIIOVEC       $ff8d
#define   SETST          $ff90
#define   SECLISTEN      $ff93
#define   SECTALK        $ff96
#define   RAMEND         $ff99
#define   RAMSTART       $ff9c
#define   GETKEY         $ff9f
#define   IECTIMEOUT     $ffa2
#define   IECIN          $ffa5
#define   IECOUT         $ffa8
#define   UNTALK         $ffab
#define   UNLISTEN       $ffae
#define   LISTEN         $ffb1
#define   TALK           $ffb4
#define   GETST          $ffb7
#define   SETFPAR        $ffba
#define   SETFNPAR       $ffbd
#define   OPEN           $ffc0
#define   CLOSE          $ffc3
#define   CHKIN          $ffc6
#define   CKOUT          $ffc9
#define   CLRCH          $ffcc
#define   BASIN          $ffcf
#define   BSOUT          $ffd2
#define   LOAD           $ffd5
#define   SAVE           $ffd8
#define   SETTI          $ffdb
#define   GETTI          $ffde
#define   GETSTP         $ffe1
#define   GET            $ffe4
#define   CLALL          $ffe7
#define   INCTI          $ffea
#define   SCREEN         $ffed
#define   CURSOR         $fff0
#define   GETIOBASE      $fff3


/*        Terminal-Commands        */

#define   TC_SCO         8
#define   TC_SCF         9

#define   TC_LF          13            /*10*/
#define   TC_CR          13

#define   TC_LCH         $0e

#define   TC_REV         18

#define   TC_F1          $85
#define   TC_F3          $86
#define   TC_F5          $87
#define   TC_F7          $88
#define   TC_F2          $89
#define   TC_F4          $8a
#define   TC_F6          $8b
#define   TC_F8          $8c

#define   TC_HCH         $8e

#define   TC_REO         $92
#define   TC_FF          $93

#define   TC_HELLGRUEN   $99

#define   TC_CRL         $9d

