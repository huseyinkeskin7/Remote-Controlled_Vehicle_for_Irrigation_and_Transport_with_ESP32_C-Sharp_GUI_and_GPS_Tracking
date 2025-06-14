using System;
using System.Globalization; // CultureInfo sınıfını ekleyin
using System.IO.Ports;  // Seri port sınıfı
using System.Drawing;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.Web.WebView2.Core;

namespace Control_center
{
    public partial class Form1 : Form
    {
        private Timer timer1;
        private SerialPort serialPort; // Seri port nesnesi
        private double latitudeValue = 0.0;
        private double longitudeValue = 0.0;
        private double distanceValue = 0.0;

        public Form1()
        {
            InitializeComponent();

            // Timer'ı başlat
            timer1 = new Timer();
            timer1.Interval = 1000; // 1 saniye
            timer1.Tick += Timer1_Tick; // Tick olayına metod bağlama
            timer1.Start(); // Timer'ı başlat

            // Seri portu başlat
            InitializeSerialPort();

            // Uygulama çalıştığında WebView2'yi başlat ve olayları bağla
            Load += async (sender, e) => await InitializeAsync();
        }

        // Seri portu başlatma
        private void InitializeSerialPort()
        {
            serialPort = new SerialPort();
            serialPort.PortName = "COM3"; // ESP32'nin bağlı olduğu COM portu
            serialPort.BaudRate = 115200; // ESP32 ile aynı baud rate
            serialPort.DataBits = 8; // Veri bit uzunluğu
            serialPort.StopBits = StopBits.One; // Durma bitleri
            serialPort.Parity = Parity.None; // Parite bitleri yok
            serialPort.Handshake = Handshake.None; // El sıkışma yok

            // Seri port aç
            serialPort.Open();

            // Veri alındığında çağrılacak olay
            serialPort.DataReceived += SerialPort_DataReceived;
        }

        // Seri porttan veri alındığında tetiklenecek olay
        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                // Gelen veriyi oku
                string receivedData = serialPort.ReadExisting(); // Daha güvenli okuma

                // UI güncellemesi için Invoke kullan
                Invoke(new Action(() =>
                {
                    // Gelen veri satırlarını ayır
                    string[] lines = receivedData.Split(new[] { "\r\n", "\n" }, StringSplitOptions.RemoveEmptyEntries);

                    foreach (var line in lines)
                    {
                        if (line.Contains("Latitude:"))
                        {
                            string latitudeStr = line.Replace("Latitude:", "").Trim();
                            if (double.TryParse(latitudeStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double lat))
                            {
                                latitudeValue = lat;
                                label7.Text = $"Latitude: {latitudeValue}";
                            }
                        }
                        else if (line.Contains("Longitude:"))
                        {
                            string longitudeStr = line.Replace("Longitude:", "").Trim();
                            if (double.TryParse(longitudeStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double lon))
                            {
                                longitudeValue = lon;
                                label8.Text = $"Longitude: {longitudeValue}";
                            }
                        }
                        else if (line.Contains("Distance:"))
                        {
                            string distanceStr = line.Replace("Distance:", "").Trim();
                            if (double.TryParse(distanceStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double dist))
                            {
                                distanceValue = dist;
                                label6.Text = $"Distance: {distanceValue}";
                                textBox1.Text = distanceValue.ToString(CultureInfo.InvariantCulture);

                                // Mesafe değerine göre buton renklerini değiştir
                                UpdateButtonColors(distanceValue);
                            }
                        }
                    }

                    // Harita güncellemesi
                    ShowMap(webView22, latitudeValue, longitudeValue);
                }));
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Hata: {ex.Message}", "Serial Port Hatası", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void UpdateButtonColors(double distance)
        {
            if (distance >= 3 && distance < 7)
            {
                button3.BackColor = Color.Red;
                button4.BackColor = Color.Yellow;
                button5.BackColor = Color.Yellow;
                button6.BackColor = Color.Green;
            }
            else if (distance >= 7 && distance < 10)
            {
                button4.BackColor = Color.Yellow;
                button3.BackColor = Color.Silver;
                button5.BackColor = Color.Yellow;
                button6.BackColor = Color.Green;
            }
            else if (distance >= 10 && distance < 15)
            {
                button5.BackColor = Color.Yellow;
                button3.BackColor = Color.Silver;
                button4.BackColor = Color.Silver;
                button6.BackColor = Color.Green;
            }
            else if (distance >= 15 && distance < 20)
            {
                button6.BackColor = Color.Green;
                button3.BackColor = Color.Silver;
                button4.BackColor = Color.Silver;
                button5.BackColor = Color.Silver;
            }
            else
            {
                button6.BackColor = Color.Silver;
                button3.BackColor = Color.Silver;
                button4.BackColor = Color.Silver;
                button5.BackColor = Color.Silver;
            }
        }

        private async Task InitializeAsync()
        {
            await InitializeWebView(webView22);
            ShowMap(webView22, latitudeValue, longitudeValue);
        }

        private async Task InitializeWebView(WebView2 webView)
        {
            if (webView.CoreWebView2 == null)
            {
                await webView.EnsureCoreWebView2Async(null);
            }
        }

        private void ShowMap(WebView2 webView, double? latitude = null, double? longitude = null)
        {
            string url;

            if (latitude.HasValue && longitude.HasValue)
            {
                url = $"https://www.google.com/maps?q={latitude.Value.ToString(CultureInfo.InvariantCulture)},{longitude.Value.ToString(CultureInfo.InvariantCulture)}";
            }
            else
            {
                url = "https://www.google.com/maps";
            }

            if (webView.CoreWebView2 != null)
            {
                webView.CoreWebView2.Navigate(url);
            }
            else
            {
                MessageBox.Show("WebView2 başlatılamadı. Lütfen WebView2 bileşenini kontrol edin.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            tabControl1.SelectedTab = tabControl1.TabPages[1];
        }

        private void pictureBox2_Click(object sender, EventArgs e)
        {
            tabControl1.SelectedTab = tabControl1.TabPages[2];
        }

        private void button1_Click(object sender, EventArgs e)
        {
            tabControl1.SelectedTab = tabControl1.TabPages[0];
        }

        private void button2_Click(object sender, EventArgs e)
        {
            tabControl1.SelectedTab = tabControl1.TabPages[0];
        }

        private void Timer1_Tick(object sender, EventArgs e)
        {
            CultureInfo englishCulture = new CultureInfo("en-US");
            label4.Text = DateTime.Now.ToString("D", englishCulture);
            label5.Text = DateTime.Now.ToString("T", englishCulture);
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
            }
            base.OnFormClosing(e);
        }
    }
}
