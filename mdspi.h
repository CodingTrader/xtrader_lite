#pragma once
#include "StdAfx.h"

class CtpMdSpi : public CThostFtdcMdSpi
{
public:
 	CtpMdSpi(CThostFtdcMdApi* api,TThostFtdcBrokerIDType bkrid):m_pApi(api),m_iReqId(0)
	{ strcpy(m_sBkrID,bkrid); };
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnFrontDisconnected(int nReason);

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///��������Ӧ��
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///����ѯ��Ӧ��
	virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///ȡ������ѯ��Ӧ��
	virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///ѯ��֪ͨ
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);
	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	void ReqUserLogin(/*TThostFtdcBrokerIDType	appId*/);
	void ReqUserLogout();

	void SubscribeForQt(char *ppInstrumentID[], int nCount);
	void UnSubscribeForQt(char *ppInstrumentID[], int nCount);
	void SubscribeMarketData(char *ppInstrumentID[], int nCount);
	void UnSubscribeMarketData(char *ppInstrumentID[], int nCount);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
public:
	int m_iReqId;
	TThostFtdcBrokerIDType	m_sBkrID;
private:
  CThostFtdcMdApi* m_pApi;
};
