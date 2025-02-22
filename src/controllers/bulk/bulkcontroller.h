#pragma once

#include <QAtomicInt>
#include <QThread>
#include <optional>

#include "controllers/controller.h"
#include "controllers/hid/legacyhidcontrollermapping.h"

struct libusb_device_handle;
struct libusb_context;

/// USB Bulk controller backend
class BulkReader : public QThread {
    Q_OBJECT
  public:
    BulkReader(libusb_device_handle *handle, unsigned char in_epaddr);
    virtual ~BulkReader();

    void stop();

  signals:
    void incomingData(const QByteArray& data, mixxx::Duration timestamp);

  protected:
    void run();

  private:
    libusb_device_handle* m_phandle;
    QAtomicInt m_stop;
    unsigned char m_in_epaddr;
};

class BulkController : public Controller {
    Q_OBJECT
  public:
    BulkController(
            libusb_context* context,
            libusb_device_handle* handle,
            struct libusb_device_descriptor* desc);
    ~BulkController() override;

    QString mappingExtension() override;

    virtual std::shared_ptr<LegacyControllerMapping> cloneMapping() override;
    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override;

    bool isMappable() const override {
        if (!m_pMapping) {
            return false;
        }
        return m_pMapping->isMappable();
    }

    bool matchMapping(const MappingInfo& mapping) override;

  protected:
    void send(const QList<int>& data, unsigned int length) override;

  private slots:
    int open() override;
    int close() override;

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    bool sendBytes(const QByteArray& data) override;

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_context;
    libusb_device_handle *m_phandle;

    // Local copies of things we need from desc

    std::uint16_t m_vendorId;
    std::uint16_t m_productId;
    std::uint8_t m_inEndpointAddr;
    std::uint8_t m_outEndpointAddr;
    std::optional<std::uint8_t> m_interfaceNumber;

    QString m_manufacturer;
    QString m_product;

    QString m_sUID;
    BulkReader* m_pReader;
    std::shared_ptr<LegacyHidControllerMapping> m_pMapping;
};
