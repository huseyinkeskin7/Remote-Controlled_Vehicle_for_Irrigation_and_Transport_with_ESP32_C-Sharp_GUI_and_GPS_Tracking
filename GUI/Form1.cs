using System;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.Web.WebView2.Core;

namespace Control_center
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            // Uygulama çalıştığında WebView2'yi başlat ve olayları bağla
            Load += async (sender, e) => await InitializeAsync();
        }

        // WebView2 başlatma ve olayları bağlama
        private async Task InitializeAsync()
        {
            await InitializeWebView(webView22);

            // WebView22'de İstanbul'u göster
            ShowMap(webView22, 41.00, 28.97);

        }

        // WebView2 bileşenini başlatma
        private async Task InitializeWebView(WebView2 webView)
        {
            if (webView.CoreWebView2 == null)
            {
                await webView.EnsureCoreWebView2Async(null);
            }
        }

        // Harita gösterme fonksiyonu
        private void ShowMap(WebView2 webView, double? latitude = null, double? longitude = null)
        {
            string url;

            // Eğer koordinatlar verilmişse, o konumu göster
            if (latitude.HasValue && longitude.HasValue)
            {
                url = $"https://www.google.com/maps?q={latitude},{longitude}";
            }
            else
            {
                // Koordinat verilmezse Google Maps ana sayfasını yükle
                url = "https://www.google.com/maps";
            }

            // WebView'i oluşturulan URL'ye yönlendir
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
    }
}
