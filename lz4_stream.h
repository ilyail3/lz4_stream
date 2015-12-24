// LZ4 Headers
#include <lz4frame.h>

// Standard headers
#include <streambuf>
#include <iostream>
#include <vector>
#include <memory>

/**
 * @brief An output stream that will LZ4 compress the input data.
 *
 * An output stream that will wrap another output stream and LZ4
 * compress its input data to that stream.
 *
 */
class LZ4OutputStream : public std::ostream
{
 public:
  /**
   * @brief Constructs an LZ4 compression output stream
   *
   * @param sink The stream to write compressed data to
   */
  LZ4OutputStream(std::ostream& sink)
    : buffer_(std::make_unique<LZ4OuputBuffer>(sink))
  {
    rdbuf(buffer_.get());
  }

  /**
   * @brief Destroys the LZ4 output stream. Calls close() if not already called.
   */
  ~LZ4OutputStream()
  {
    close();
  }

  /**
   * @brief Flushes and writes LZ4 footer data to the LZ4 output stream.
   *
   * After calling this function no more data should be written to the stream.
   */
  void close()
  {
    buffer_->close();
  }

 private:
  class LZ4OuputBuffer : public std::streambuf
  {
  public:
    LZ4OuputBuffer(std::ostream &sink);
    ~LZ4OuputBuffer();

    LZ4OuputBuffer(const LZ4OuputBuffer &) = delete;
    LZ4OuputBuffer& operator= (const LZ4OuputBuffer &) = delete;

  private:
    int_type overflow(int_type ch) override;
    int_type sync() override;

    void compressAndWrite();
    void writeHeader();
    void writeFooter();

    std::ostream& sink_;
    std::array<char, 256> src_buf_;
    std::vector<char> dest_buf_;
    LZ4F_compressionContext_t ctx_;
  };

  std::unique_ptr<LZ4OuputBuffer> buffer_;
};

/**
 * @brief An input stream that will LZ4 decompress output data.
 *
 * An input stream that will wrap another input stream and LZ4
 * decompress its output data to that stream.
 *
 */
class LZ4InputStream : public std::istream
{
 public:
  /**
   * @brief Constructs an LZ4 decompression input stream
   *
   * @param source The stream to read LZ4 compressed data from
   */
  LZ4InputStream(std::istream& source)
    : buffer_(std::make_unique<LZ4InputBuffer>(source))
  {
    rdbuf(buffer_.get());
  }

 private:
  class LZ4InputBuffer : public std::streambuf
  {
  public:
    LZ4InputBuffer(std::istream &source);
    ~LZ4InputBuffer();
    int_type underflow() override;

    LZ4InputBuffer(const LZ4InputBuffer &) = delete;
    LZ4InputBuffer& operator= (const LZ4InputBuffer &) = delete;
  private:
    std::istream& source_;
    std::vector<char> src_buf_;
    std::vector<char> dest_buf_;
    size_t offset_;
    size_t src_buf_size_;
    LZ4F_decompressionContext_t ctx_;
  };

  std::unique_ptr<LZ4InputBuffer> buffer_;
};