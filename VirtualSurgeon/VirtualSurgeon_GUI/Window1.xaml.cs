using System;
using System.Windows;
using System.Windows.Media.Imaging;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.IO;
using System.Collections.Generic;
using VirtualSurgeon_CSWrapper;

namespace VirtualSurgeon_GUI
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private VirtualSurgeonWrapper m_vs_wrapper;

        public Window1()
        {
            try 
	        {
                InitializeComponent();

                m_vs_wrapper = new VirtualSurgeonWrapper();
	        }
	        catch (Exception e)
	        {
                System.Console.WriteLine(e.Message);
	        }
        }

        private BitmapSource myBitmapImage;
        private string image_filename;
        private string model_filename;

        private void load_from_file(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Image"; // Default file name
            //dlg.DefaultExt = ".txt"; // Default file extension
            //dlg.Filter = "Text documents (.txt)|*.txt"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
                image_filename = dlg.FileName;

                // Create source
                BitmapImage bi = new BitmapImage();
                bi.BeginInit();
                bi.UriSource = new Uri(image_filename);
                bi.EndInit();

                myBitmapImage = bi;

                Point[] _ps = load_points(image_filename);

                image1.Source = myBitmapImage;
                image1.UnderlayImage = myBitmapImage;

                if (_ps != null)
                {
                    image1.points = _ps;
                    image1.Init();
                }

                image1.InvalidateVisual();

                m_vs_wrapper.Initialize(image_filename);

                propertyGrid.Instance = m_vs_wrapper.getParamsWrapper();
            }

        }

        private delegate void NoArgDelegate();
        private delegate void ExtractHeadDelegate(BitmapSource _bs);
        private void button2_Click(object sender, RoutedEventArgs e)
        {
            //System.Threading.Thread t = new System.Threading.Thread(
            //    new System.Threading.ThreadStart(doExtractHead));
            //t.IsBackground = true;
            //label1.Content = "Extracting...";
            //DoubleAnimation a = new DoubleAnimation(0, 100, TimeSpan.FromSeconds(5));
            //a.RepeatBehavior = RepeatBehavior.Forever;
            //a.AutoReverse = true;
            //progressBar1.BeginAnimation(System.Windows.Controls.ProgressBar.ValueProperty, a);

            this.Dispatcher.BeginInvoke(
                new NoArgDelegate(doExtractHead),
                System.Windows.Threading.DispatcherPriority.Background);

            
            //t.Start();
            //extract head
            //m_vs_wrapper.ExtractHead();
        }

        private void doExtractHead()
        {
            
            //System.Threading.Thread t = new System.Threading.Thread(new System.Threading.ThreadStart(m_vs_wrapper.ExtractHead));
            //t.IsBackground = true;
            //t.Start();
            //t.Join();
            //button2.Dispatcher.BeginInvoke(
            //    new ExtractHeadDelegate(m_vs_wrapper.ExtractHead),
            //    System.Windows.Threading.DispatcherPriority.Background);
            m_vs_wrapper.ExtractHead();

            SetMaskAsImage();
            radio_mask.IsChecked = true;
        }

        //extract neck button
        private void neck_extract(object sender, RoutedEventArgs e)
        {
            VirtualSurgeonPoint[] points = m_vs_wrapper.FindNeck();
            Point[] newp = new Point[points.Length];
            int i = 0;
            foreach (VirtualSurgeonPoint p in points)
            {
                newp.SetValue(new Point(p.x, p.y), i++);
            }
            image1.points = newp;
            image1.UnderlayImage = myBitmapImage;
            image1.Init();
            image1.InvalidateVisual();
        }

        //load from URL
        private void load_from_url(object sender, RoutedEventArgs e)
        {
            SimpleTextDialog std = new SimpleTextDialog();
            if (std.ShowDialog() == true)
            {
                m_vs_wrapper.Initialize(std.ResponseText);

                image_filename = m_vs_wrapper.GetFilename();

                BitmapSource bs = SetOrigAsImage();
                if (bs == null)
                {
                    MessageBox.Show("Cannot get image to display");
                    return;
                }

                myBitmapImage = bs;

                propertyGrid.Instance = m_vs_wrapper.getParamsWrapper();
            }
        }

        private void load_model(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                model_filename = dlg.FileName;

                m_vs_wrapper.InitializeModel(model_filename);

                // Create source
                //BitmapImage bi = new BitmapImage();
                //bi.BeginInit();
                //bi.UriSource = new Uri(filename);
                //bi.EndInit();

                //image2.Source = bi;
                //image2.UnderlayImage = bi;

                SetOriginalModel();

                Point[] _points = load_points(model_filename);
                if (_points == null)
                {
                    MessageBox.Show("Cannot load neck points for model");
                }
                else
                {
                    image2.points = _points;
                    image2.Init();
                }

                image2.InvalidateVisual();
            }
        }

        private Point[] load_points(string filename_of_image)
        {
            Stream s = null;
            try
            {
                s = File.Open(
                    filename_of_image.Substring(0, filename_of_image.LastIndexOf('.')) + ".points",
                    FileMode.Open
                );
            }
            catch (FileNotFoundException)
            {
                return null;
            }
            if (s.CanRead)
            {
                StreamReader sr = new StreamReader(s);
                List<Point> points_in_filr = new List<Point>();
                while (!sr.EndOfStream)
                {
                    Point p = Point.Parse(sr.ReadLine());
                    points_in_filr.Add(p);
                }
                return points_in_filr.ToArray();
            }
            else
            {
                return null;
            }
        }

        private void save_neck(object sender, RoutedEventArgs e)
        {
            Point[] ps = image1.points;
            Stream s = File.Open(
                image_filename.Substring(0,image_filename.LastIndexOf('.')) + ".points",
                FileMode.Create
                );
            StreamWriter sw = new StreamWriter(s);
            foreach (Point p in ps)
            {
                sw.WriteLine(p);
            }
            sw.Flush();
            sw.Close();

            FileStream stream = new FileStream(image_filename.Substring(0, image_filename.LastIndexOf('.')) + ".mask.bin", 
                FileMode.OpenOrCreate,FileAccess.Write);
            //PngBitmapEncoder encoder = new PngBitmapEncoder();
            
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage i = m_vs_wrapper.getHeadMask();
            if (i == null) return;

            BitmapSource bs = BitmapSource.Create(
                i.width, i.height, 96, 96,
                System.Windows.Media.PixelFormats.Gray32Float,
                null, i.buf, i.row_stride);

            //encoder.Frames.Add(BitmapFrame.Create(bs));
            //encoder.Save(stream);
            stream.Write(i.buf, 0, i.buf_size);
            stream.Close();
        }

        private void warp_model(object sender, RoutedEventArgs e)
        {
            VirtualSurgeonPoint[] face_points = new VirtualSurgeonPoint[8];
            VirtualSurgeonPoint[] model_points = new VirtualSurgeonPoint[8];
            for (int i = 0; i < 8; i++)
            {
                face_points[i] = new VirtualSurgeonPoint();
                face_points[i].x = (int)image1.points[i+2].X;
                face_points[i].y = (int)image1.points[i+2].Y;
                model_points[i] = new VirtualSurgeonPoint();
                model_points[i].x = (int)image2.points[i+2].X;
                model_points[i].y = (int)image2.points[i+2].Y;
            }

            m_vs_wrapper.Warp(face_points, model_points);

            SetWarpedModel();
        }

        private void make_model(object sender, RoutedEventArgs e)
        {
            VirtualSurgeonPoint p = new VirtualSurgeonPoint();
            VirtualSurgeonPoint p1 = new VirtualSurgeonPoint();
            p.x = (int)((image1.points[5].X + image1.points[6].X) / 2.0);
            p1.x = (int)((image2.points[5].X + image2.points[6].X)/2.0);
            p.y = (int)((image1.points[5].Y + image1.points[6].Y) / 2.0);
            p1.y = (int)((image2.points[5].Y + image2.points[6].Y) / 2.0);
            m_vs_wrapper.MakeModel(p,p1);

            SetCompleteModel();
        }

        private void recolor_model(object sender, RoutedEventArgs e)
        {
            m_vs_wrapper.Recolor();
            SetRecoloredModel();
        }

        #region Setting window images to data from wrapper
        private BitmapSource SetOrigAsImage()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage i = m_vs_wrapper.getOrigImage();
            if (i == null) return null;

            BitmapSource bs = BitmapSource.Create(
                i.width, i.height, 96, 96,
                System.Windows.Media.PixelFormats.Bgr24,
                null, i.buf, i.row_stride);
            Dispatcher.Invoke(
                (ExtractHeadDelegate)delegate(BitmapSource _bs)
                {
                    image1.UnderlayImage = _bs;
                    image1.Init();
                    image1.InvalidateVisual();
                },
                bs);
            return bs;
        }
        private void SetMaskAsImage()
        {
            //display image
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage i = m_vs_wrapper.getHeadMask();
            if (i == null) return;

            BitmapSource bs = BitmapSource.Create(
                i.width, i.height, 96, 96,
                System.Windows.Media.PixelFormats.Gray32Float,
                null, i.buf, i.row_stride);
            Dispatcher.Invoke(
                (ExtractHeadDelegate)delegate(BitmapSource _bs)
                {
                    image1.Source = _bs;
                },
                bs);
        }
        private void radio_mask_Checked(object sender, RoutedEventArgs e)
        {
            SetMaskAsImage();
            radio_mask.IsChecked = true;
        }

        private void radio_neck_Checked(object sender, RoutedEventArgs e)
        {
            SetOrigAsImage();
            radio_neck.IsChecked = true;
        }
        private void set_original_model(object sender, RoutedEventArgs e)
        {
            SetOriginalModel();
            radio_model_orig.IsChecked = true;
        }
        private void set_nohead_model(object sender, RoutedEventArgs e)
        {
            SetNoHeadModel();
            radio_nohead.IsChecked = true;
        }
        private void set_warped_model(object sender, RoutedEventArgs e)
        {
            SetWarpedModel();
            radio_warped.IsChecked = true;
        }
        private void set_complete_model(object sender, RoutedEventArgs e)
        {
            SetCompleteModel();
            radio_complete.IsChecked = true;
        }
        private void set_recolored_model(object sender, RoutedEventArgs e)
        {
            SetRecoloredModel();
        }

        void SetCompleteModel()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage im = m_vs_wrapper.getCompleteModel();
            if (im == null) return;
            SetModelSource(im);
        }

        void SetWarpedModel()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage im = m_vs_wrapper.getWarpedModel();
            if (im == null) return;
            SetModelSource(im);
        }

        void SetNoHeadModel()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage im = m_vs_wrapper.getModelNoHead();
            if (im == null) return;
            SetModelSource(im);
        }
        void SetRecoloredModel()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage im = m_vs_wrapper.getModelRecolored();
            if (im == null) return;
            SetModelSource(im);
        }

        private void SetModelSource(VirtualSurgeon_CSWrapper.VirtualSurgeonImage im)
        {
            BitmapSource bs = BitmapSource.Create(
                im.width, im.height, 96, 96,
                System.Windows.Media.PixelFormats.Bgr24,
                null, im.buf, im.row_stride);
            Dispatcher.Invoke(
                (ExtractHeadDelegate)delegate(BitmapSource _bs)
                {
                    image2.Source = _bs;
                    image2.UnderlayImage = _bs;
                    image2.InvalidateVisual();
                },
                bs);
        }

        void SetOriginalModel()
        {
            VirtualSurgeon_CSWrapper.VirtualSurgeonImage im = m_vs_wrapper.getModelImage();
            if (im == null) return;
            SetModelSource(im);
            image2.Init();
        } 

        #endregion

        private void button9_Click(object sender, RoutedEventArgs e)
        {
            image2.ShowMidPoint();
            image2.InvalidateVisual();
        }

        private void save_completed(object sender, RoutedEventArgs e)
        {
            int pos = model_filename.LastIndexOf('\\') + 1;
            string fname = model_filename.Substring(pos, model_filename.LastIndexOf('.')-pos);
            pos = image_filename.LastIndexOf('\\') + 1;
            fname += "_" + image_filename.Substring(pos, image_filename.LastIndexOf('.')-pos) + ".png";
            FileStream stream5 = new FileStream(fname, FileMode.Create);
            PngBitmapEncoder encoder5 = new PngBitmapEncoder();
            encoder5.Frames.Add(BitmapFrame.Create(image2.UnderlayImage));
            encoder5.Save(stream5);
        }
    }


    public partial class PointsImage : Image
    {
        public Point[] points { get; set; }
        public BitmapSource UnderlayImage { get; set; }
        GeometryDrawing mousePointGeometryDrwaing;
        GeometryGroup pointsGeometryGroup;
        GeometryGroup linesGeometryGroup;
        GeometryGroup middlePointGeoGrp;
        int chosenPoint;

        public void Init()
        {
            DrawingGroup dg = new DrawingGroup();
            ImageDrawing id = new ImageDrawing(UnderlayImage, new Rect(0, 0, UnderlayImage.PixelWidth, UnderlayImage.PixelHeight));
            dg.Children.Add(id);

            pointsGeometryGroup = new GeometryGroup();
            linesGeometryGroup = new GeometryGroup();
            middlePointGeoGrp = new GeometryGroup();
            if (points != null)
            {
                SetPointsGeometry();
            }

            GeometryDrawing gd = new GeometryDrawing(Brushes.Blue, null, pointsGeometryGroup);
            dg.Children.Add(gd);

            GeometryDrawing gd2 = new GeometryDrawing(null, new Pen(Brushes.LightGreen,3), linesGeometryGroup);
            dg.Children.Add(gd2);

            GeometryDrawing gd1 = new GeometryDrawing(Brushes.Red, null, middlePointGeoGrp);
            dg.Children.Add(gd1);

            Brush b = new SolidColorBrush(Colors.Red);
            b.Opacity = 0.5;
            mousePointGeometryDrwaing = new GeometryDrawing(b, null, null);
            dg.Children.Add(mousePointGeometryDrwaing);

            DrawingImage di = new DrawingImage(dg);
            this.Source = di;

            chosenPoint = -1;
        }

        public void ShowMidPoint()
        {
            DrawingGroup dg = new DrawingGroup();
            ImageDrawing id = new ImageDrawing(UnderlayImage, new Rect(0, 0, UnderlayImage.PixelWidth, UnderlayImage.PixelHeight));
            dg.Children.Add(id);
            GeometryDrawing gd1 = new GeometryDrawing(Brushes.Red, null, middlePointGeoGrp);
            dg.Children.Add(gd1);
            DrawingImage di = new DrawingImage(dg);
            this.Source = di;
        }

        private void SetPointsGeometry()
        {
            pointsGeometryGroup.Children.Clear();
            linesGeometryGroup.Children.Clear();
            middlePointGeoGrp.Children.Clear();

            if (points == null) return;

            for (int i = 0; i < points.Length; i++)
            {
                Point p = points[i];
                pointsGeometryGroup.Children.Add(new EllipseGeometry(p, 4, 4));
                if (i > 0)
                {
                    Point p_1 = points[i - 1];
                    linesGeometryGroup.Children.Add(new LineGeometry(p_1, p));
                }
            }
            Point tmpP = new Point();
            tmpP.X = (points[points.Length / 2 - 1].X + points[points.Length / 2].X) / 2.0;
            tmpP.Y = (points[points.Length / 2 - 1].Y + points[points.Length / 2].Y) / 2.0;
            middlePointGeoGrp.Children.Add(new EllipseGeometry(tmpP, 8, 8));

        }

        Point start;
        Point origin;

        protected override void OnMouseDown(System.Windows.Input.MouseButtonEventArgs e)
        {
            base.OnMouseDown(e);
            Point p_relative = getRelativePoint(e.GetPosition(this));

            this.CaptureMouse();
            TranslateTransform tt = (TranslateTransform)((TransformGroup)this.RenderTransform).Children[1];

            if (points != null)
            {
                for (int i = 0; i < points.Length; i++)
                {
                    Point p = points[i];
                    double l = Point.Subtract(p, p_relative).LengthSquared;
                    //Console.WriteLine(p + "-" + p_relative + " = " + l);
                    if (l < 25)
                    {
                        //this point was clicked
                        Console.WriteLine("chosen point " + i);
                        chosenPoint = i;
                    }
                }
            }

            start = e.GetPosition((Border)this.Parent);
            if (chosenPoint < 0)
            {
                origin = new Point(tt.X, tt.Y);
            }
        }

        protected override void OnMouseMove(System.Windows.Input.MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if (mousePointGeometryDrwaing != null)
            {
                Point origp = e.GetPosition(this);
                Point p = getRelativePoint(origp);
                mousePointGeometryDrwaing.Geometry = new EllipseGeometry(p, 10, 10);
                if (chosenPoint >= 0)
                {
                    points[chosenPoint].X = p.X;
                    points[chosenPoint].Y = p.Y;
                }
                SetPointsGeometry();
                this.InvalidateVisual();
            }
            if (chosenPoint < 0)
            {
                if (this.IsMouseCaptured)
                {
                    var tt = (TranslateTransform)((TransformGroup)this.RenderTransform)
                        .Children[1];
                    Vector v = start - e.GetPosition((Border)this.Parent);
                    tt.X = origin.X - v.X;
                    tt.Y = origin.Y - v.Y;
                }
            }
        }

        protected override void OnMouseUp(System.Windows.Input.MouseButtonEventArgs e)
        {
            base.OnMouseUp(e);
            if(chosenPoint >= 0)
                chosenPoint = -1;

            this.ReleaseMouseCapture();
        }

        protected override void OnMouseLeave(System.Windows.Input.MouseEventArgs e)
        {
            base.OnMouseLeave(e);
            if(mousePointGeometryDrwaing!= null)
                mousePointGeometryDrwaing.Geometry = null;//remove red dot
        }

        protected override void OnMouseWheel(System.Windows.Input.MouseWheelEventArgs e)
        {
            base.OnMouseWheel(e);

            ScaleTransform st = (ScaleTransform)((TransformGroup)this.RenderTransform).Children[0];
            double zoom = e.Delta > 0 ? .05 : -.05;
            st.ScaleX += zoom;
            st.ScaleY += zoom;
        }

        private Point getRelativePoint(Point p)
        {
            return new Point(
                p.X / this.ActualWidth * UnderlayImage.PixelWidth,
                p.Y / this.ActualHeight * UnderlayImage.PixelHeight);
        }
    }
}
