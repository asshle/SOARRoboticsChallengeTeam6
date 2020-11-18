namespace Robotics_Testing
{
    partial class lb_SerialReturn
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.button2 = new System.Windows.Forms.Button();
            this.tb_SerialReturn = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(163, 12);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(400, 120);
            this.button2.TabIndex = 1;
            this.button2.Text = "Send Command";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // tb_SerialReturn
            // 
            this.tb_SerialReturn.Location = new System.Drawing.Point(163, 205);
            this.tb_SerialReturn.Multiline = true;
            this.tb_SerialReturn.Name = "tb_SerialReturn";
            this.tb_SerialReturn.Size = new System.Drawing.Size(484, 217);
            this.tb_SerialReturn.TabIndex = 2;
            // 
            // lb_SerialReturn
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.tb_SerialReturn);
            this.Controls.Add(this.button2);
            this.Name = "lb_SerialReturn";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.TextBox tb_SerialReturn;
    }
}

