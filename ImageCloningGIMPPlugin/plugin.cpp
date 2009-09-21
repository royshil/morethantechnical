#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <vector>
#include <gtk/gtk.h>

#include "ImageEditingUtils.h"

using namespace std;

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const GimpParam  *param,
                   gint             *nreturn_vals,
                   GimpParam       **return_vals);
static void doClone(GimpDrawable *drawable,GimpDrawable* backgroundDrawable,int);
static gboolean blur_dialog (GimpDrawable *drawable);

typedef struct _ImageCloningVars {
	gint shouldUseMixedGradient;
} ImageCloningVars;

static ImageCloningVars image_cloning_vars = {false};

int get_laplace_for_pixel(guchar* row, guchar* nextrow, guchar* prevrow, int x, int y, int color, int channels, int m, int n);

class GimpIImage : public ImageEditingUtils::IImage {
	GimpPixelRgn m_rgn_in, m_rgn_out;
	GimpDrawable* m_drawable;
	int m_channels;
	int x_offs,y_offs;

public:
	GimpIImage(GimpDrawable* drawable, int x1, int y1, int width, int height) {
		m_drawable = drawable;
		m_channels = gimp_drawable_bpp (drawable->drawable_id);
		gimp_pixel_rgn_init (&m_rgn_in,drawable,x1,y1,width,height,FALSE, FALSE);
		gimp_pixel_rgn_init (&m_rgn_out,drawable,x1,y1,width,height,TRUE, TRUE);
		x_offs = x1;
		y_offs = y1;

		fprintf(stdout,"new GimpIImage(%dx%dx%d), drawable %X, region (%d,%d)->(%d,%d)\n",width,height,m_channels,(unsigned int)drawable,x1,y1,x1+width,y1+height);
	}
	~GimpIImage() {
//		gimp_drawable_flush(m_drawable);
	}

	int getRGB(int x,int y) {
		guchar pxl[4];
		gimp_pixel_rgn_get_pixel (&m_rgn_in,pxl,x+x_offs,y+y_offs);
		int rgb = 0;
		if(m_channels == 1) {
			rgb = pxl[0] | ((int)pxl[0]) << 8 | ((int)pxl[0]) << 16;
		} else if(m_channels == 3 || m_channels == 4) {
			rgb = pxl[0] | ((int)pxl[1]) << 8 | ((int)pxl[2]) << 16;
		}
		return rgb;
	}
	void setRGB(int x,int y,int rgb) {
		if(m_channels == 1) {
			guchar pxl = (guchar)(rgb & 0x000000ff);
			gimp_pixel_rgn_set_pixel(&m_rgn_out,&pxl,x+x_offs,y+y_offs);
		} else if(m_channels == 3) {
			guchar pxl[3] = {0};
			pxl[0] = (guchar)(rgb & 0x000000ff);
			pxl[1] = (guchar)((rgb & 0x0000ff00) >> 8);
			pxl[2] = (guchar)((rgb & 0x00ff0000) >> 16);
			gimp_pixel_rgn_set_pixel(&m_rgn_out,pxl,x+x_offs,y+y_offs);
		} else if(m_channels == 4) {
			guchar pxl[4] = {0};
			pxl[0] = (guchar)(rgb & 0x000000ff);
			pxl[1] = (guchar)((rgb & 0x0000ff00) >> 8);
			pxl[2] = (guchar)((rgb & 0x00ff0000) >> 16);
			gimp_pixel_rgn_set_pixel(&m_rgn_out,pxl,x+x_offs,y+y_offs);
		}
	}
};

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  query,
  run
};

MAIN()

static void
query (void)
{
  static GimpParamDef args[] =
  {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Run mode"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Input image"
    },
    {
      GIMP_PDB_DRAWABLE,
      "drawable",
      "Input drawable"
    }
  };

  gimp_install_procedure (
    "plug-in-image-cloning",
    "Image cloning",
    "Inserts the top layer on to the bottom layer using linear system solving",
    "Roy Shil",
    "GPL",
    "2009",
    "_Image_Cloning",
    "RGB*, GRAY*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args), 0,
    args, NULL);

  gimp_plugin_menu_register ("plug-in-image-cloning",
                             "<Image>/Filters/Misc");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam  values[1];
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  GimpRunMode       run_mode;

  /* Setting mandatory output values */
  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  /* Getting run_mode - we won't display a dialog if
   * we are in NONINTERACTIVE mode */
  run_mode = (GimpRunMode)param[0].data.d_int32;
  gimp_progress_init ("Clone...");
  GimpDrawable* drawable = gimp_drawable_get (param[2].data.d_drawable);
  int num_layers;
  gint* layers = gimp_image_get_layers(param[1].data.d_image,&num_layers);
  fprintf(stdout,"num layers : %d\n",num_layers);

  GimpDrawable* backgroundDrawable = gimp_drawable_get(layers[1]);
  fprintf(stdout,"drawable: %X, background drawable: %X\n",(unsigned int)drawable,(unsigned int)backgroundDrawable);

  gboolean dialog_response = true;
  switch (run_mode)
  {
	case GIMP_RUN_INTERACTIVE:
		dialog_response = blur_dialog(drawable);
	break;

	case GIMP_RUN_NONINTERACTIVE:
	break;

	case GIMP_RUN_WITH_LAST_VALS:
	default:
	break;
  }

  if(dialog_response) {
	doClone(drawable,backgroundDrawable,param[1].data.d_image);
  }

  gimp_progress_end();
  gimp_displays_flush ();
	gimp_drawable_detach (backgroundDrawable);
}

static gboolean
blur_dialog (GimpDrawable *drawable)
{
	GtkWidget *dialog;
	GtkWidget *main_vbox;
	GtkWidget *main_hbox;
	GtkWidget *frame;
	GtkWidget *radius_label;
	GtkWidget *alignment;
	GtkWidget *spinbutton;
	GtkObject *spinbutton_adj;
	GtkWidget *frame_label;
	gboolean   run;

	gimp_ui_init ("image_cloning", FALSE);
	
	dialog = gimp_dialog_new ("Image Cloning", "image_cloning",
							  NULL, GtkDialogFlags::GTK_DIALOG_MODAL,
							  gimp_standard_help_func, "plug-in-image-cloning",
							  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							  GTK_STOCK_OK,     GTK_RESPONSE_OK,
							  NULL);

	main_vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), main_vbox);
	gtk_widget_show (main_vbox);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 6, 6, 6);

	main_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (main_hbox);
	gtk_container_add (GTK_CONTAINER (alignment), main_hbox);

	radius_label = gtk_label_new_with_mnemonic ("Use _Mixed gradients:");
	gtk_widget_show (radius_label);
	gtk_box_pack_start (GTK_BOX (main_hbox), radius_label, FALSE, FALSE, 6);
	gtk_label_set_justify (GTK_LABEL (radius_label), GTK_JUSTIFY_RIGHT);

	spinbutton = gtk_check_button_new();
	gtk_widget_show (spinbutton);
	gtk_box_pack_start (GTK_BOX (main_hbox), spinbutton, FALSE, FALSE, 6);

	frame_label = gtk_label_new ("Gradient Type");
	gtk_widget_show (frame_label);
	gtk_frame_set_label_widget (GTK_FRAME (frame), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);

	g_signal_connect (spinbutton, "toggled",
					  G_CALLBACK (gimp_toggle_button_update),
					  &image_cloning_vars.shouldUseMixedGradient);
	gtk_widget_show (dialog);

	run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

	gtk_widget_destroy (dialog);

	return run;
}


static void doClone(GimpDrawable *drawable,GimpDrawable* backgroundDrawable, gint32 image_id) {
	gint         channels, mask_channels, back_channels;
	gint         x1, y1, x2, y2, x_off, y_off;
	guchar       *row, *nextrow, *prevrow, *backrow, *maskrow, *backnextrow, *backprevrow;
	gint32 		 mask_id;
	GimpPixelRgn rgn_in, rgn_mask_in, rgn_back_in;
//	char 		charbuf[512] = {0};

	gimp_drawable_mask_bounds (drawable->drawable_id,
	                                   &x1, &y1,
	                                   &x2, &y2);
	gimp_drawable_offsets(drawable->drawable_id,&x_off,&y_off);

	fprintf(stdout,"do clone from (%d,%d) to (%d,%d)\n",x_off,y_off,x2+x_off,y2+y_off);

	if(image_cloning_vars.shouldUseMixedGradient) {
		fprintf(stdout,"use mixed gradients\n");
	}

	mask_id = gimp_layer_get_mask(drawable->drawable_id);
	GimpDrawable* mask_drawable = gimp_drawable_get(mask_id);
	channels = gimp_drawable_bpp (drawable->drawable_id);
	mask_channels = gimp_drawable_bpp(mask_drawable->drawable_id);
	back_channels = gimp_drawable_bpp(backgroundDrawable->drawable_id);

	int m = y2 - y1;
	int n = x2 - x1;
	int mn = m * n;

	gimp_pixel_rgn_init (&rgn_in,
						 drawable,
						 x1, y1,
						 n, m,
						 FALSE, FALSE);
//	gimp_pixel_rgn_init (&rgn_out,
//						 drawable,
//						 x1, y1,
//						 n, m,
//						 TRUE, TRUE);
	gimp_pixel_rgn_init (&rgn_mask_in,
						 mask_drawable,
						 x1, y1,
						 n, m,
						 FALSE, FALSE);
	gimp_pixel_rgn_init (&rgn_back_in,
						 backgroundDrawable,
						 x_off, y_off,
						 n, m,
						 FALSE, FALSE);

	row = g_new (guchar, channels * n);
	nextrow = g_new (guchar, channels * n);
	prevrow = g_new (guchar, channels * n);

	maskrow = g_new (guchar, mask_channels * n);

	backrow = g_new (guchar, back_channels * n);
	backnextrow = g_new (guchar, back_channels * n);
	backprevrow = g_new (guchar, back_channels * n);

	std::vector<double > rgbVector[3];// = {std::vector<double >(mn)};
	for (int color = 0; color < 3; color++) {
		rgbVector[color] = std::vector<double >(mn);
	}

	for (int y = 0; y < m; y++) {
//		fprintf(stdout,"line: %d (%d->%d)\n",y,y1,y2);

		gimp_pixel_rgn_get_row (&rgn_in, 		row, 		0, y, n);
		if(y < m-1) { //next row available
			gimp_pixel_rgn_get_row (&rgn_in, 	nextrow,	0, y+1, n);
			gimp_pixel_rgn_get_row (&rgn_back_in, 	backnextrow, 	x_off, y_off + y+1, n);
		}
		if(y > 0) { //prev row available
			gimp_pixel_rgn_get_row (&rgn_in, 	prevrow,	0, y-1, n);
			gimp_pixel_rgn_get_row (&rgn_back_in, 	backprevrow, 	x_off, y_off + y-1, n);
		}
		gimp_pixel_rgn_get_row (&rgn_back_in, 	backrow, 	x_off, y_off + y, n);
		gimp_pixel_rgn_get_row (&rgn_mask_in, 	maskrow, 	0, y, n);

		for (int x = 0; x < n; x++) {
			for (int color = 0; color < 3; color++) {
				guchar maskRGB = maskrow[mask_channels * x];
				if(maskRGB == 0) {
					rgbVector[color][y*n+x] = backrow[back_channels * x + color];
				} else {
					//int clonedLaplaceForPixel = 0;
					//int numNeighbours = 0;

					////right pixel
					//if (x < n - 1) {
					//	clonedLaplaceForPixel += row[channels * (x + 1) + color];
					//	numNeighbours++;
					//}

					////below pixel
					//if (y < m - 1) {
					//	clonedLaplaceForPixel += nextrow[channels * x + color];
					//	numNeighbours++;
					//}

					////left pixel
					//if (x > 0) {
					//	clonedLaplaceForPixel += row[channels * (x - 1) + color];
					//	numNeighbours++;
					//}

					////top pixel
					//if (y > 0) {
					//	clonedLaplaceForPixel += prevrow[channels * x + color];
					//	numNeighbours++;
					//}

					//clonedLaplaceForPixel -= row[channels * x + color] * numNeighbours;

					int g_laplace = get_laplace_for_pixel(row,nextrow,prevrow,x,y,color,channels,m,n);

					if(image_cloning_vars.shouldUseMixedGradient) {
						int f_laplace = get_laplace_for_pixel(backrow,backnextrow,backprevrow,x,y,color,back_channels,m,n);

						////if use mixed gradients
						if(abs(f_laplace) > abs(g_laplace)) {
							rgbVector[color][y * n + x] = f_laplace;
						} else {
							rgbVector[color][y * n + x] = g_laplace;
						}
					} else {
						rgbVector[color][y * n + x] = g_laplace;
					}
				}
			}
		}
	}

	gmm::row_matrix< gmm::rsvector<double> > M(mn,mn);

	gimp_progress_set_text_printf("Create matrix %dx%d",mn,mn);
	gimp_progress_update(0.33);

	GimpIImage maskImage(mask_drawable,x1,y1,n,m);
	ImageEditingUtils::matrixCreate(M, n, mn, maskImage);

	std::vector<double > solutionVectors[3];
	for (int color = 0; color < 3; color++) {
		solutionVectors[color] = std::vector<double >(mn);
	}

//	gint32 new_layer = gimp_layer_new_from_drawable(backgroundDrawable->drawable_id,image_id);
	gint32 new_layer = gimp_layer_new(image_id,"output",n,m,GIMP_RGB_IMAGE,100.0,GIMP_NORMAL_MODE);
	GimpDrawable* new_drawable = gimp_drawable_get(new_layer);
	GimpPixelRgn rgn_output_out;
	gimp_pixel_rgn_init(&rgn_output_out,new_drawable,0,0,n,m,TRUE,TRUE);
	for(int y=0;y<m;y++) {
		gimp_pixel_rgn_get_row(&rgn_back_in, backrow, x_off, y_off + y, n);
		gimp_pixel_rgn_set_row(&rgn_output_out, backrow, 0, y, n);
	}
	GimpIImage outputImage(new_drawable,0,0,n,m);
//	GimpIImage outputImage(new_drawable,x_off,y_off,n,m);

	gimp_progress_set_text_printf("solve linear system");
	gimp_progress_update(0.66);

	for (int color = 0; color < 3; color++) {
		ImageEditingUtils::solveLinear(M,solutionVectors[color],rgbVector[color]);
		gimp_progress_set_text_printf("done with color %d",color);
		gimp_progress_update(0.66 + 0.11*color);
	}
	for(int y=0;y<m;y++)
		for(int x=0;x<n;x++)
		{
			guchar pxl[3] = {0};
			pxl[0] = max(min(solutionVectors[0][y*n + x],255.0),0.0);
			pxl[1] = max(min(solutionVectors[1][y*n + x],255.0),0.0);
			pxl[2] = max(min(solutionVectors[2][y*n + x],255.0),0.0);
			gimp_pixel_rgn_set_pixel(&rgn_output_out,pxl,x,y);
		}

//	ImageEditingUtils::solveAndPaintOutput(0, 0, n, mn, rgbVector, M,
//			solutionVectors, outputImage);

	gimp_layer_translate(new_layer,x_off,y_off);

	gimp_image_add_layer(image_id,new_layer,-1);
	gimp_layer_remove_mask(new_layer,GIMP_MASK_DISCARD);
	gimp_drawable_flush(new_drawable);
	gimp_drawable_merge_shadow (new_drawable->drawable_id, TRUE);
	gimp_drawable_update (new_drawable->drawable_id,
						  x_off, y_off, n, m);

	gimp_progress_update(1.0);

	g_free (row);
	g_free (nextrow);
	g_free (prevrow);
	g_free (backrow);
	g_free (backnextrow);
	g_free (backprevrow);
	g_free (maskrow);
//	g_free (outrow);

	gimp_drawable_flush (backgroundDrawable);
	gimp_drawable_flush (mask_drawable);
	gimp_drawable_flush (drawable);
}

int get_laplace_for_pixel(guchar* row, guchar* nextrow, guchar* prevrow, int x, int y, int color, int channels, int m, int n) {
	int clonedLaplaceForPixel = 0;
	int numNeighbours = 0;

	//right pixel
	if (x < n - 1) {
		clonedLaplaceForPixel += row[channels * (x + 1) + color];
		numNeighbours++;
	}

	//below pixel
	if (y < m - 1) {
		clonedLaplaceForPixel += nextrow[channels * x + color];
		numNeighbours++;
	}

	//left pixel
	if (x > 0) {
		clonedLaplaceForPixel += row[channels * (x - 1) + color];
		numNeighbours++;
	}

	//top pixel
	if (y > 0) {
		clonedLaplaceForPixel += prevrow[channels * x + color];
		numNeighbours++;
	}

	clonedLaplaceForPixel -= row[channels * x + color] * numNeighbours;

	return clonedLaplaceForPixel;
}