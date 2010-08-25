using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace OpenCVInterop
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            OpenCVImageWrapper im = OpenCVImageWrapper.getFromOpenCV();

            BitmapSource bs = BitmapSource.Create(
                            im.width, im.height, 96, 96,
                           System.Windows.Media.PixelFormats.Bgr24,
                            null, im.buf, im.row_stride);

            imageOnScreen.Source = bs;

        }
    }
}
