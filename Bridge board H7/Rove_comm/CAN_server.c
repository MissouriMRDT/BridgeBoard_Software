#include "CAN_server.h"
#include "main.h"

void RC_CAN_init(FDCAN_HandleTypeDef *hfdcan1, uint8_t ID){

	FDCAN_FilterTypeDef Filterconfig = {0};

	Filterconfig.IdType = FDCAN_EXTENDED_ID;
	Filterconfig.FilterIndex =0;
	Filterconfig.FilterType = FDCAN_FILTER_MASK;
	Filterconfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

	CAN_ID_h filter_ID = {.raw =0};
	CAN_ID_h filter_mask = {.raw =0};

	filter_ID.header.dest = ID;
	filter_mask.header.dest = 0x0F;

	Filterconfig.FilterID1 = filter_ID.raw;
	Filterconfig.FilterID2 = filter_mask.raw;

	if(HAL_FDCAN_ConfigFilter(hfdcan1, &Filterconfig)!=HAL_OK){
		 Error_Handler();
	}

	if(HAL_FDCAN_ConfigGlobalFilter(hfdcan1,FDCAN_REJECT,FDCAN_REJECT,FDCAN_FILTER_REMOTE,FDCAN_FILTER_REMOTE) != HAL_OK){
		Error_Handler();
	}

	if(HAL_FDCAN_ActivateNotification(hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0) != HAL_OK){
		Error_Handler();
	}

	if (HAL_FDCAN_Start(hfdcan1) != HAL_OK)
	  {
	      Error_Handler();
	  }
}

void RC_CAN_send(FDCAN_HandleTypeDef *hfdcan1,CAN_ID_h header){
	FDCAN_TxHeaderTypeDef hdr = {0};
	uint8_t dummy_data[4]     = {0};

		hdr.Identifier = header.raw & 0x1FFFFFFFU;
		hdr.IdType = FDCAN_EXTENDED_ID;
		hdr.TxFrameType = FDCAN_DATA_FRAME;
		hdr.DataLength = FDCAN_DLC_BYTES_4;
		hdr.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
		hdr.BitRateSwitch = FDCAN_BRS_OFF;
		hdr.FDFormat = FDCAN_CLASSIC_CAN;  //TODO replace once new chips in
		hdr.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
		hdr.MessageMarker = 0;

		if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &hdr, dummy_data) != HAL_OK) {

		        Error_Handler();
		    }
}

void RC_CAN_Receive(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
	FDCAN_RxHeaderTypeDef rx_header;
	uint8_t data[8];// temp add payload union here

	if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, data) ==HAL_OK){
		CAN_ID_h rx_ID;
		rx_ID.raw=rx_header.Identifier;


	}

}
