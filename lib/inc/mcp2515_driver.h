/*
 * mcp2515_driver.h
 *
 *  Created on: 18 Jun 2021
 *      Author: mehmedblazevic
 * Description: This file contains functions needed to control
 * the MCP2515 from Microchip. Every commands to control the
 * MCP are implemented and a CAN abstraction is provided which
 * allow the user to send and receive CAN messages with ease.
 */

#ifndef MCP2515_DRIVER_H_
#define MCP2515_DRIVER_H_

#include <ti/drivers/SPI.h>
#include <ti/drivers/GPIO.h>
#include <stdint.h>
//#include <math.h>
#include <ti/drivers/dpl/ClockP.h>
#include <string.h>
#include "ti_drivers_config.h"


/* Configuration Registers */
#define CANSTAT         0x0E
#define CANCTRL         0x0F
#define BFPCTRL         0x0C
#define TEC             0x1C
#define REC             0x1D
#define CNF3            0x28
#define CNF2            0x29
#define CNF1            0x2A
#define CANINTE         0x2B
#define CANINTF         0x2C
#define EFLG            0x2D
#define TXRTSCTRL       0x0D

/*  Recieve Filters */
#define RXF0SIDH        0x00
#define RXF0SIDL        0x01
#define RXF0EID8        0x02
#define RXF0EID0        0x03
#define RXF1SIDH        0x04
#define RXF1SIDL        0x05
#define RXF1EID8        0x06
#define RXF1EID0        0x07
#define RXF2SIDH        0x08
#define RXF2SIDL        0x09
#define RXF2EID8        0x0A
#define RXF2EID0        0x0B
#define RXF3SIDH        0x10
#define RXF3SIDL        0x11
#define RXF3EID8        0x12
#define RXF3EID0        0x13
#define RXF4SIDH        0x14
#define RXF4SIDL        0x15
#define RXF4EID8        0x16
#define RXF4EID0        0x17
#define RXF5SIDH        0x18
#define RXF5SIDL        0x19
#define RXF5EID8        0x1A
#define RXF5EID0        0x1B

/* Receive Masks */
#define RXM0SIDH        0x20
#define RXM0SIDL        0x21
#define RXM0EID8        0x22
#define RXM0EID0        0x23
#define RXM1SIDH        0x24
#define RXM1SIDL        0x25
#define RXM1EID8        0x26
#define RXM1EID0        0x27

/* Tx Buffer 0 */
#define TXB0CTRL        0x30
#define TXB0SIDH        0x31
#define TXB0SIDL        0x32
#define TXB0EID8        0x33
#define TXB0EID0        0x34
#define TXB0DLC         0x35
#define TXB0D0          0x36
#define TXB0D1          0x37
#define TXB0D2          0x38
#define TXB0D3          0x39
#define TXB0D4          0x3A
#define TXB0D5          0x3B
#define TXB0D6          0x3C
#define TXB0D7          0x3D

/* Tx Buffer 1 */
#define TXB1CTRL        0x40
#define TXB1SIDH        0x41
#define TXB1SIDL        0x42
#define TXB1EID8        0x43
#define TXB1EID0        0x44
#define TXB1DLC         0x45
#define TXB1D0          0x46
#define TXB1D1          0x47
#define TXB1D2          0x48
#define TXB1D3          0x49
#define TXB1D4          0x4A
#define TXB1D5          0x4B
#define TXB1D6          0x4C
#define TXB1D7          0x4D

/* Tx Buffer 2 */
#define TXB2CTRL        0x50
#define TXB2SIDH        0x51
#define TXB2SIDL        0x52
#define TXB2EID8        0x53
#define TXB2EID0        0x54
#define TXB2DLC         0x55
#define TXB2D0          0x56
#define TXB2D1          0x57
#define TXB2D2          0x58
#define TXB2D3          0x59
#define TXB2D4          0x5A
#define TXB2D5          0x5B
#define TXB2D6          0x5C
#define TXB2D7          0x5D

/* Rx Buffer 0 */
#define RXB0CTRL        0x60
#define RXB0SIDH        0x61
#define RXB0SIDL        0x62
#define RXB0EID8        0x63
#define RXB0EID0        0x64
#define RXB0DLC         0x65
#define RXB0D0          0x66
#define RXB0D1          0x67
#define RXB0D2          0x68
#define RXB0D3          0x69
#define RXB0D4          0x6A
#define RXB0D5          0x6B
#define RXB0D6          0x6C
#define RXB0D7          0x6D

/* Rx Buffer 1 */
#define RXB1CTRL        0x70
#define RXB1SIDH        0x71
#define RXB1SIDL        0x72
#define RXB1EID8        0x73
#define RXB1EID0        0x74
#define RXB1DLC         0x75
#define RXB1D0          0x76
#define RXB1D1          0x77
#define RXB1D2          0x78
#define RXB1D3          0x79
#define RXB1D4          0x7A
#define RXB1D5          0x7B
#define RXB1D6          0x7C
#define RXB1D7          0x7D


/*******************************************************************
 *               Bit register masks                                *
 *******************************************************************/

/* TXBnCTRL */
#define TXREQ           0x08
#define TXP             0x03

/* RXBnCTRL */
#define RXM             0x60
#define BUKT            0x04

/* CANCTRL */
#define REQOP           0xE0
#define ABAT            0x10
#define OSM             0x08
#define CLKEN           0x04
#define CLKPRE          0x03

/* CANSTAT */
#define REQOP           0xE0
#define ICOD            0x0E

/* CANINTE */
#define RX0IE           0x01
#define RX1IE           0x02
#define TX0IE           0x04
#define TX1IE           0x80
#define TX2IE           0x10
#define ERRIE           0x20
#define WAKIE           0x40
#define MERRE           0x80

/* CANINTF */
#define RX0IF           0x01
#define RX1IF           0x02
#define TX0IF           0x04
#define TX1IF           0x80
#define TX2IF           0x10
#define ERRIF           0x20
#define WAKIF           0x40
#define MERRF           0x80

/* BFPCTRL */
#define B1BFS           0x20
#define B0BFS           0x10
#define B1BFE           0x08
#define B0BFE           0x04
#define B1BFM           0x02
#define B0BFM           0x01

/* CNF1 Masks */
#define SJW             0xC0
#define BRP             0x3F

/* CNF2 Masks */
#define BTLMODE         0x80
#define SAM             0x40
#define PHSEG1          0x38
#define PRSEG           0x07

/* CNF3 Masks */
#define WAKFIL          0x40
#define PHSEG2          0x07

/* TXRTSCTRL Masks */
#define TXB2RTS         0x04
#define TXB1RTS         0x02
#define TXB0RTS         0x01

/* RXBnSIDL Masks */
#define SRR     0x10
#define IDE     0x08


/*******************************************************************
 *                    Bit Timing Configuration                     *
 *******************************************************************/

/* CNF1 */
#define SJW_1TQ         0x40
#define SJW_2TQ         0x80
#define SJW_3TQ         0x90
#define SJW_4TQ         0xC0

/* CNF2 */
#define BTLMODE_CNF3    0x80
#define BTLMODE_PH1_IPT 0x00

#define SMPL_3X         0x40
#define SMPL_1X         0x00

#define PHSEG1_8TQ      0x38
#define PHSEG1_7TQ      0x30
#define PHSEG1_6TQ      0x28
#define PHSEG1_5TQ      0x20
#define PHSEG1_4TQ      0x18
#define PHSEG1_3TQ      0x10
#define PHSEG1_2TQ      0x08
#define PHSEG1_1TQ      0x00

#define PRSEG_8TQ       0x07
#define PRSEG_7TQ       0x06
#define PRSEG_6TQ       0x05
#define PRSEG_5TQ       0x04
#define PRSEG_4TQ       0x03
#define PRSEG_3TQ       0x02
#define PRSEG_2TQ       0x01
#define PRSEG_1TQ       0x00

/* CNF3 */
#define PHSEG2_8TQ      0x07
#define PHSEG2_7TQ      0x06
#define PHSEG2_6TQ      0x05
#define PHSEG2_5TQ      0x04
#define PHSEG2_4TQ      0x03
#define PHSEG2_3TQ      0x02
#define PHSEG2_2TQ      0x01
#define PHSEG2_1TQ      0x00

#define SOF_ENABLED     0x80
#define WAKFIL_ENABLED  0x40
#define WAKFIL_DISABLED 0x00


/*******************************************************************
 *                  Control/Configuration Registers                *
 *******************************************************************/

/* CANINTE */
#define RX0IE_ENABLED   0x01
#define RX0IE_DISABLED  0x00
#define RX1IE_ENABLED   0x02
#define RX1IE_DISABLED  0x00
#define G_RXIE_ENABLED  0x03
#define G_RXIE_DISABLED 0x00

#define TX0IE_ENABLED   0x04
#define TX0IE_DISABLED  0x00
#define TX1IE_ENABLED   0x08
#define TX2IE_DISABLED  0x00
#define TX2IE_ENABLED   0x10
#define TX2IE_DISABLED  0x00
#define G_TXIE_ENABLED  0x1C
#define G_TXIE_DISABLED 0x00

#define ERRIE_ENABLED   0x20
#define ERRIE_DISABLED  0x00
#define WAKIE_ENABLED   0x40
#define WAKIE_DISABLED  0x00
#define IVRE_ENABLED    0x80
#define IVRE_DISABLED   0x00

/* CANINTF */
#define RX0IF_SET       0x01
#define RX0IF_RESET     0x00
#define RX1IF_SET       0x02
#define RX1IF_RESET     0x00
#define TX0IF_SET       0x04
#define TX0IF_RESET     0x00
#define TX1IF_SET       0x08
#define TX2IF_RESET     0x00
#define TX2IF_SET       0x10
#define TX2IF_RESET     0x00
#define ERRIF_SET       0x20
#define ERRIF_RESET     0x00
#define WAKIF_SET       0x40
#define WAKIF_RESET     0x00
#define IVRF_SET        0x80
#define IVRF_RESET      0x00

/* CANCTRL */
#define REQOP_CONFIG    0x80
#define REQOP_LISTEN    0x60
#define REQOP_LOOPBACK  0x40
#define REQOP_SLEEP     0x20
#define REQOP_NORMAL    0x00

#define ABORT           0x10

#define OSM_ENABLED     0x08

#define CLKOUT_ENABLED  0x04
#define CLKOUT_DISABLED 0x00
#define CLKOUT_PRE_8    0x03
#define CLKOUT_PRE_4    0x02
#define CLKOUT_PRE_2    0x01
#define CLKOUT_PRE_1    0x00

/* CANSTAT */
#define OPMODE_CONFIG   0x80
#define OPMODE_LISTEN   0x60
#define OPMODE_LOOPBACK 0x40
#define OPMODE_SLEEP    0x20
#define OPMODE_NORMAL   0x00


/* RXBnCTRL */
#define RXM_RCV_ALL     0x60
#define RXM_VALID_EXT   0x40
#define RXM_VALID_STD   0x20
#define RXM_VALID_ALL   0x00

#define RXRTR_REMOTE    0x08
#define RXRTR_NO_REMOTE 0x00

#define BUKT_ROLLOVER    0x04
#define BUKT_NO_ROLLOVER 0x00

#define FILHIT0_FLTR_1  0x01
#define FILHIT0_FLTR_0  0x00

#define FILHIT1_FLTR_5  0x05
#define FILHIT1_FLTR_4  0x04
#define FILHIT1_FLTR_3  0x03
#define FILHIT1_FLTR_2  0x02
#define FILHIT1_FLTR_1  0x01
#define FILHIT1_FLTR_0  0x00


/* TXBnCTRL */
#define TXREQ_SET       0x08
#define TXREQ_CLEAR     0x00

#define TXP_HIGHEST     0x03
#define TXP_INTER_HIGH  0x02
#define TXP_INTER_LOW   0x01
#define TXP_LOWEST      0x00


/*******************************************************************
 *                  Register Bit Masks                             *
 *******************************************************************/

 #define DLC_0          0x00
 #define DLC_1          0x01
 #define DLC_2          0x02
 #define DLC_3          0x03
 #define DLC_4          0x04
 #define DLC_5          0x05
 #define DLC_6          0x06
 #define DLC_7          0x07
 #define DLC_8          0x08


/*******************************************************************
 *                  CAN SPI commands                               *
 *******************************************************************/

#define CAN_RESET       0xC0
#define CAN_READ        0x03
#define CAN_WRITE       0x02
#define CAN_RTS         0x80
#define CAN_RTS_TXB0    0x81
#define CAN_RTS_TXB1    0x82
#define CAN_RTS_TXB2    0x84
#define CAN_RD_STATUS   0xA0
#define CAN_BIT_MODIFY  0x05
#define CAN_RX_STATUS   0xB0
#define CAN_RD_RX_BUFF  0x90
#define CAN_LOAD_TX     0X40


/*******************************************************************
 *                  Miscellaneous                                  *
 *******************************************************************/

#define DUMMY_BYTE      0x00
#define TXB0            0x31
#define TXB1            0x41
#define TXB2            0x51
#define RXB0            0x61
#define RXB1            0x71
#define EXIDE_SET       0x08
#define EXIDE_RESET     0x00
#define RTR             0x06


#define CAN_500kbps     3
#define CAN_250kbps     7
#define CAN_125kbps     15
#define CAN_50kbps      39 // need some changes if you need a slower baudrate

#define SPI_DEFAULT_CLOCK   10000000

#define MY_ID   0x42

#define OK      0
#define ERR     -1


// Commands
#define INST_RESET          0xC0
#define INST_WRITE          0x02 // write data to the register beginning
#define INST_LOAD_TX_BUF    0x40 // 0100 0abc -> a,b,c: address pointer of the buffer to write to
#define INST_READ           0x03 // reads data from the register beginning
#define INST_READ_RX_BUF    0x90 // 1001 0nm0 -> n,m: address pointer of the buffer to read
#define INST_RTS            0x80 // 1000 0nnn -> n2: RTS for TXB2, n1: RTS for TXB1, 0: RTS for TXB0
#define INST_READ_STAT      0xA0 // reads several status bits for transmit and receive functions
#define INST_RX_STAT        0xB0 // indicates filter match and message type
#define INST_BIT_MOD        0x05 // set or clear individual bits in a particular register


#define SUPPORT_EXTENDED_CANID (1)
#define SUPPORT_TIMESTAMPS (0)


typedef struct
{
    #if SUPPORT_EXTENDED_CANID
        uint32_t id;                //!< ID der Nachricht (11 oder 29 Bit)
        struct {
            bool rtr;            //!< Remote-Transmit-Request-Frame?
            bool extended;       //!< extended ID?
        } flags;
    #else
        uint16_t id;                //!< ID der Nachricht (11 Bit)
        struct {
            int rtr : 1;            //!< Remote-Transmit-Request-Frame?
        } flags;
    #endif

    uint8_t length;             //!< Anzahl der Datenbytes
    uint8_t data[8];            //!< Die Daten der CAN Nachricht

    #if SUPPORT_TIMESTAMPS
        uint16_t timestamp;
    #endif
} can_t;


#define _bit_is_set(pin, bit)   (pin & (1<<bit))
#define _bit_is_clear(pin, bit) (!(pin & (1<<bit)))

/*!
 *  @brief  Initialize the MCP2515. For the CAN bitrate,
 *  please use the defines in this file (CAN_125kbps,...)
 *
 *  @param  spi_clock    The SPI clock frequency
 *  @param  can_bitrate  The CAN bitrate
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t init_mcp2515(uint32_t spi_clock, uint8_t can_bitrate);

// init only the registers of the MCP2515
void Init2515();
void mcp2515_close();

/*!
 *  @brief  Reset the MCP2515.
 */
void mcp2515_rst();
/*!
 *  @brief  The RX STATUS instruction is used to quickly determine
 *  which filter matched the message and message type (standard,
 *  extended and/or remote) of received message.
 *
 *  Note: The associated RX flag bit, RXnIF (CANINTF), will be
 *  cleared after bringing /CS high.
 *
 *  @return the value of the register
 */
char mcp2515_rx_status();
/*!
 *  @brief  When reading a receive buffer, reduces the overhead of a
 *  normal READ command by placing the Address Pointer at one of four
 *  locations, as indicated by ‘n,m’.
 *
 *  n   m       adresse points to           adress
 *  0   0       Buf 0, start @ RXB0SIDH     0x61
 *  0   1       Buf 0, start @ RXB0D0       0x66
 *  1   0       Buf 1, start @ RXB1SIDH     0x71
 *  1   1       Buf 1, start @ RXB1D0       0x76
 *
 *  @param  nm      The address pointer
 *
 *  @return the value of the register
 */
char mcp2515_rx_buffer(char nm);
/*!
 *  @brief  Quick polling command that reads several status bits for
 *  transmit and receive functions.
 *
 *  b7      b6      b5      b4      b3      b2      b1      b0
 *  TX2IF   TXREQ   TX1IF   TXREQ   TX0IF   TXREQ   RX1IF   RX0IF
 *
 *  @return the value of the register
 */
char mcp2515_read_status();
/*!
 *  @brief  Reads data from the register beginning at selected address.
 *
 *  @param  adr      The address pointer
 *  @param  datas    The pointer to store the datas read
 *  @param  n        Number of bytes to read
 *
 */
void mcp2515_read(char adr, char *datas, char n);
/*!
 *  @brief  Writes data to the register beginning at the selected address.
 *
 *  @param  datas      The datas's array to write
 *  @param  n          Number of bytes to write
 *
 */
void mcp2515_write(char *datas, char n);
/*!
 *  @brief  When loading a transmit buffer, reduces the overhead of
 *  a normal WRITE command by placing the address pointer at one
 *  of six locations, as indicated by 'a,b,c'.
 *
 *  a   b   c       address points to               addr
 *  0   0   0       TX buf 0, start @ TXB0SIDH      0x31
 *  0   0   1       TX buf 0, start @ TXB0D0        0x36
 *  0   1   0       TX buf 1, start @ TXB1SIDH      0x41
 *  0   1   1       TX buf 1, start @ TXB1D0        0x46
 *  1   0   0       TX buf 2, start @ TXB2SIDH      0x51
 *  1   0   1       TX buf 2, start @ TXB2D0        0x56
 *
 *  @param  abc     The address pointer
 *  @param  data    The data to send
 *
 */
void mcp2515_load_tx_buffer(char abc, char data);
/*!
 *  @brief  Instructs controller to begin message transmission
 *  sequence for any of the transmit buffers.
 *
 *      n    n    n
 *    TXB2  TXB1 TXB0
 *
 *  @param  nnn      The address pointer
 *
 */
void mcp2515_RTS(char nnn);


/*!
 *  @brief  Clear all interrupt
 *
 */
void mcp2515_clearInt();


/*!
 *  @brief  Allows the user to set or clear individuals bits in
 *  a particular register.
 *
 *  @param  adr      The address of the register
 *  @param  mask     The mask byte
 *  @param  data     The data to write
 *
 *  @return the value of the register
 */
void mcp2515_bit_modify(char adr, char mask, char data);


/*!
 *  @brief  This function sends a CAN message through the MCP.
 *
 *  @param  msg      The CAN message to send
 *
 *  @return the address of the tx buffer
 */
unsigned char mcp2515_send_message(can_t *msg);
/*!
 *  @brief  Check if we received a CAN message
 *
 *  @return true if a CAN message received or false
 */
bool mcp2515_check_message();
/*!
 *  @brief  This function reads a CAN message from the MCP.
 *
 *  @param  msg      Store the CAN message here
 *
 *  @return the status register
 */
unsigned char mcp2515_get_message(can_t *msg);

#if SUPPORT_EXTENDED_CANID
/*!
 *  @brief  Read the CAN id of the received message and store it
 *  in the *id parameter. CAUTION: chip select MUST be asserted!
 *
 *  @param  id      Address to store the id received
 *
 *  @return the status register
 */
unsigned char mcp2515_read_id(uint32_t *id); // chip select should be asserted
/*!
 *  @brief  Write the CAN id for the transmit message.
 *  CAUTION: chip select MUST be asserted!
 *
 *  @param  id      the point to the id value
 *
 */
void mcp2515_write_id(uint32_t *id, uint8_t extended);         // same
#else
/*!
 *  @brief  Read the CAN id of the received message and store it
 *  in the *id parameter. CAUTION: chip select MUST be asserted!
 *
 *  @param  id      Address to store the id received
 *
 *  @return the status register
 */
unsigned char mcp2515_read_id(uint16_t *id); // chip select should be asserted
/*!
 *  @brief  Write the CAN id for the transmit message.
 *  CAUTION: chip select MUST be asserted!
 *
 *  @param  id      the point to the id value
 *
 */
void mcp2515_write_id(uint16_t *id);         // same
#endif


#endif /* MCP2515_DRIVER_H_ */
