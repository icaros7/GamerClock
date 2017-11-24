<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form은 Dispose를 재정의하여 구성 요소 목록을 정리합니다.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Windows Form 디자이너에 필요합니다.
    Private components As System.ComponentModel.IContainer

    '참고: 다음 프로시저는 Windows Form 디자이너에 필요합니다.
    '수정하려면 Windows Form 디자이너를 사용하십시오.  
    '코드 편집기에서는 수정하지 마세요.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(Form1))
        Me.Label1 = New System.Windows.Forms.Label()
        Me.Hour = New System.Windows.Forms.Label()
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Min = New System.Windows.Forms.Label()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.Sec = New System.Windows.Forms.Label()
        Me.Label6 = New System.Windows.Forms.Label()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Hours = New System.Windows.Forms.ComboBox()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.Mins = New System.Windows.Forms.ComboBox()
        Me.Label7 = New System.Windows.Forms.Label()
        Me.Secs = New System.Windows.Forms.ComboBox()
        Me.GroupBox1 = New System.Windows.Forms.GroupBox()
        Me.TextBox1 = New System.Windows.Forms.TextBox()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.Label8 = New System.Windows.Forms.Label()
        Me.CopyLeft = New System.Windows.Forms.LinkLabel()
        Me.RadioButton1 = New System.Windows.Forms.RadioButton()
        Me.RadioButton2 = New System.Windows.Forms.RadioButton()
        Me.RadioButton3 = New System.Windows.Forms.RadioButton()
        Me.RadioButton4 = New System.Windows.Forms.RadioButton()
        Me.ListBox1 = New System.Windows.Forms.ListBox()
        Me.Label9 = New System.Windows.Forms.Label()
        Me.Processs = New System.Windows.Forms.Label()
        Me.NotifyIcon1 = New System.Windows.Forms.NotifyIcon(Me.components)
        Me.ContextMenuStrip1 = New System.Windows.Forms.ContextMenuStrip(Me.components)
        Me.App_Name = New System.Windows.Forms.ToolStripMenuItem()
        Me.Open_Form = New System.Windows.Forms.ToolStripMenuItem()
        Me.ToolStripSeparator1 = New System.Windows.Forms.ToolStripSeparator()
        Me.Exit_Form = New System.Windows.Forms.ToolStripMenuItem()
        Me.AOC = New System.Windows.Forms.CheckBox()
        Me.RTSSHook = New System.Windows.Forms.CheckBox()
        Me.Timer2 = New System.Windows.Forms.Timer(Me.components)
        Me.search = New System.Windows.Forms.TextBox()
        Me.Label10 = New System.Windows.Forms.Label()
        Me.Button2 = New System.Windows.Forms.Button()
        Me.ListBox2 = New System.Windows.Forms.ListBox()
        Me.Button3 = New System.Windows.Forms.Button()
        Me.TextBox2 = New System.Windows.Forms.TextBox()
        Me.Re_OSD = New System.Windows.Forms.ToolStripMenuItem()
        Me.GroupBox1.SuspendLayout()
        Me.ContextMenuStrip1.SuspendLayout()
        Me.SuspendLayout()
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Font = New System.Drawing.Font("맑은 고딕", 15.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label1.Location = New System.Drawing.Point(12, 9)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(116, 30)
        Me.Label1.TabIndex = 0
        Me.Label1.Text = "현재 시각 :"
        '
        'Hour
        '
        Me.Hour.AutoSize = True
        Me.Hour.Font = New System.Drawing.Font("맑은 고딕", 18.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Hour.Location = New System.Drawing.Point(127, 9)
        Me.Hour.Name = "Hour"
        Me.Hour.Size = New System.Drawing.Size(43, 32)
        Me.Hour.TabIndex = 1
        Me.Hour.Text = "00"
        '
        'Timer1
        '
        Me.Timer1.Enabled = True
        Me.Timer1.Interval = 1000
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label2.Location = New System.Drawing.Point(162, 20)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(21, 17)
        Me.Label2.TabIndex = 2
        Me.Label2.Text = "시"
        '
        'Min
        '
        Me.Min.AutoSize = True
        Me.Min.Font = New System.Drawing.Font("맑은 고딕", 18.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Min.Location = New System.Drawing.Point(189, 9)
        Me.Min.Name = "Min"
        Me.Min.Size = New System.Drawing.Size(43, 32)
        Me.Min.TabIndex = 1
        Me.Min.Text = "00"
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label4.Location = New System.Drawing.Point(224, 20)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(21, 17)
        Me.Label4.TabIndex = 2
        Me.Label4.Text = "분"
        '
        'Sec
        '
        Me.Sec.AutoSize = True
        Me.Sec.Font = New System.Drawing.Font("맑은 고딕", 18.0!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Sec.Location = New System.Drawing.Point(251, 9)
        Me.Sec.Name = "Sec"
        Me.Sec.Size = New System.Drawing.Size(43, 32)
        Me.Sec.TabIndex = 1
        Me.Sec.Text = "00"
        '
        'Label6
        '
        Me.Label6.AutoSize = True
        Me.Label6.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label6.Location = New System.Drawing.Point(286, 20)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(21, 17)
        Me.Label6.TabIndex = 2
        Me.Label6.Text = "초"
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label3.Location = New System.Drawing.Point(82, 26)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(21, 17)
        Me.Label3.TabIndex = 2
        Me.Label3.Text = "시"
        '
        'Hours
        '
        Me.Hours.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.Hours.Font = New System.Drawing.Font("맑은 고딕", 11.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Hours.FormattingEnabled = True
        Me.Hours.ImeMode = System.Windows.Forms.ImeMode.Off
        Me.Hours.Items.AddRange(New Object() {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"})
        Me.Hours.Location = New System.Drawing.Point(36, 20)
        Me.Hours.Name = "Hours"
        Me.Hours.Size = New System.Drawing.Size(46, 28)
        Me.Hours.Sorted = True
        Me.Hours.TabIndex = 3
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label5.Location = New System.Drawing.Point(155, 26)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(21, 17)
        Me.Label5.TabIndex = 2
        Me.Label5.Text = "분"
        '
        'Mins
        '
        Me.Mins.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.Mins.Font = New System.Drawing.Font("맑은 고딕", 11.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Mins.FormattingEnabled = True
        Me.Mins.ImeMode = System.Windows.Forms.ImeMode.Off
        Me.Mins.Location = New System.Drawing.Point(109, 20)
        Me.Mins.Name = "Mins"
        Me.Mins.Size = New System.Drawing.Size(46, 28)
        Me.Mins.TabIndex = 3
        '
        'Label7
        '
        Me.Label7.AutoSize = True
        Me.Label7.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Label7.Location = New System.Drawing.Point(228, 26)
        Me.Label7.Name = "Label7"
        Me.Label7.Size = New System.Drawing.Size(21, 17)
        Me.Label7.TabIndex = 2
        Me.Label7.Text = "초"
        '
        'Secs
        '
        Me.Secs.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
        Me.Secs.Font = New System.Drawing.Font("맑은 고딕", 11.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.Secs.FormattingEnabled = True
        Me.Secs.ImeMode = System.Windows.Forms.ImeMode.Off
        Me.Secs.Location = New System.Drawing.Point(182, 20)
        Me.Secs.Name = "Secs"
        Me.Secs.Size = New System.Drawing.Size(46, 28)
        Me.Secs.Sorted = True
        Me.Secs.TabIndex = 3
        '
        'GroupBox1
        '
        Me.GroupBox1.Controls.Add(Me.Secs)
        Me.GroupBox1.Controls.Add(Me.Label3)
        Me.GroupBox1.Controls.Add(Me.Mins)
        Me.GroupBox1.Controls.Add(Me.Label5)
        Me.GroupBox1.Controls.Add(Me.Hours)
        Me.GroupBox1.Controls.Add(Me.Label7)
        Me.GroupBox1.Location = New System.Drawing.Point(14, 124)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(295, 67)
        Me.GroupBox1.TabIndex = 4
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "종료 예정 시간"
        '
        'TextBox1
        '
        Me.TextBox1.Location = New System.Drawing.Point(14, 70)
        Me.TextBox1.Name = "TextBox1"
        Me.TextBox1.Size = New System.Drawing.Size(297, 21)
        Me.TextBox1.TabIndex = 4
        '
        'Button1
        '
        Me.Button1.Location = New System.Drawing.Point(197, 492)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(113, 31)
        Me.Button1.TabIndex = 5
        Me.Button1.Text = "시작"
        Me.Button1.UseVisualStyleBackColor = True
        '
        'Label8
        '
        Me.Label8.AutoSize = True
        Me.Label8.Location = New System.Drawing.Point(15, 52)
        Me.Label8.Name = "Label8"
        Me.Label8.Size = New System.Drawing.Size(231, 12)
        Me.Label8.TabIndex = 5
        Me.Label8.Text = "메시지 (RTSS 사용시 한국어 깨짐 주의) :"
        '
        'CopyLeft
        '
        Me.CopyLeft.AutoSize = True
        Me.CopyLeft.Font = New System.Drawing.Font("맑은 고딕", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(129, Byte))
        Me.CopyLeft.Location = New System.Drawing.Point(8, 498)
        Me.CopyLeft.Name = "CopyLeft"
        Me.CopyLeft.Size = New System.Drawing.Size(135, 17)
        Me.CopyLeft.TabIndex = 6
        Me.CopyLeft.TabStop = True
        Me.CopyLeft.Text = "Copyleft ⓒ hominlab"
        '
        'RadioButton1
        '
        Me.RadioButton1.AutoSize = True
        Me.RadioButton1.Location = New System.Drawing.Point(12, 102)
        Me.RadioButton1.Name = "RadioButton1"
        Me.RadioButton1.Size = New System.Drawing.Size(63, 16)
        Me.RadioButton1.TabIndex = 7
        Me.RadioButton1.Text = "30분 뒤"
        Me.RadioButton1.UseVisualStyleBackColor = True
        '
        'RadioButton2
        '
        Me.RadioButton2.AutoSize = True
        Me.RadioButton2.Location = New System.Drawing.Point(87, 102)
        Me.RadioButton2.Name = "RadioButton2"
        Me.RadioButton2.Size = New System.Drawing.Size(69, 16)
        Me.RadioButton2.TabIndex = 7
        Me.RadioButton2.Text = "1시간 뒤"
        Me.RadioButton2.UseVisualStyleBackColor = True
        '
        'RadioButton3
        '
        Me.RadioButton3.AutoSize = True
        Me.RadioButton3.Location = New System.Drawing.Point(162, 102)
        Me.RadioButton3.Name = "RadioButton3"
        Me.RadioButton3.Size = New System.Drawing.Size(69, 16)
        Me.RadioButton3.TabIndex = 7
        Me.RadioButton3.Text = "2시간 뒤"
        Me.RadioButton3.UseVisualStyleBackColor = True
        '
        'RadioButton4
        '
        Me.RadioButton4.AutoSize = True
        Me.RadioButton4.Checked = True
        Me.RadioButton4.Location = New System.Drawing.Point(237, 102)
        Me.RadioButton4.Name = "RadioButton4"
        Me.RadioButton4.Size = New System.Drawing.Size(59, 16)
        Me.RadioButton4.TabIndex = 7
        Me.RadioButton4.TabStop = True
        Me.RadioButton4.Text = "커스텀"
        Me.RadioButton4.UseVisualStyleBackColor = True
        '
        'ListBox1
        '
        Me.ListBox1.FormattingEnabled = True
        Me.ListBox1.ItemHeight = 12
        Me.ListBox1.Location = New System.Drawing.Point(11, 225)
        Me.ListBox1.Name = "ListBox1"
        Me.ListBox1.Size = New System.Drawing.Size(300, 220)
        Me.ListBox1.Sorted = True
        Me.ListBox1.TabIndex = 8
        '
        'Label9
        '
        Me.Label9.AutoSize = True
        Me.Label9.Location = New System.Drawing.Point(10, 206)
        Me.Label9.Name = "Label9"
        Me.Label9.Size = New System.Drawing.Size(105, 12)
        Me.Label9.TabIndex = 9
        Me.Label9.Text = "선택된 프로세스 : "
        '
        'Processs
        '
        Me.Processs.AutoSize = True
        Me.Processs.Location = New System.Drawing.Point(121, 206)
        Me.Processs.Name = "Processs"
        Me.Processs.Size = New System.Drawing.Size(133, 12)
        Me.Processs.TabIndex = 10
        Me.Processs.Text = "종료할 프로세스를 선택"
        '
        'NotifyIcon1
        '
        Me.NotifyIcon1.ContextMenuStrip = Me.ContextMenuStrip1
        Me.NotifyIcon1.Text = "Gamer Clock"
        Me.NotifyIcon1.Visible = True
        '
        'ContextMenuStrip1
        '
        Me.ContextMenuStrip1.ImageScalingSize = New System.Drawing.Size(28, 28)
        Me.ContextMenuStrip1.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.App_Name, Me.Open_Form, Me.Re_OSD, Me.ToolStripSeparator1, Me.Exit_Form})
        Me.ContextMenuStrip1.Name = "ContextMenuStrip1"
        Me.ContextMenuStrip1.Size = New System.Drawing.Size(211, 120)
        '
        'App_Name
        '
        Me.App_Name.Enabled = False
        Me.App_Name.Name = "App_Name"
        Me.App_Name.Size = New System.Drawing.Size(210, 22)
        Me.App_Name.Text = "Gamer Clock for RTSS v1"
        '
        'Open_Form
        '
        Me.Open_Form.Name = "Open_Form"
        Me.Open_Form.ShortcutKeys = CType((System.Windows.Forms.Keys.Control Or System.Windows.Forms.Keys.S), System.Windows.Forms.Keys)
        Me.Open_Form.Size = New System.Drawing.Size(210, 22)
        Me.Open_Form.Text = "보이기"
        '
        'ToolStripSeparator1
        '
        Me.ToolStripSeparator1.Name = "ToolStripSeparator1"
        Me.ToolStripSeparator1.Size = New System.Drawing.Size(207, 6)
        '
        'Exit_Form
        '
        Me.Exit_Form.Name = "Exit_Form"
        Me.Exit_Form.ShortcutKeys = CType((System.Windows.Forms.Keys.Control Or System.Windows.Forms.Keys.E), System.Windows.Forms.Keys)
        Me.Exit_Form.Size = New System.Drawing.Size(210, 22)
        Me.Exit_Form.Text = "종료"
        '
        'AOC
        '
        Me.AOC.AutoSize = True
        Me.AOC.Enabled = False
        Me.AOC.Location = New System.Drawing.Point(11, 471)
        Me.AOC.Name = "AOC"
        Me.AOC.Size = New System.Drawing.Size(126, 16)
        Me.AOC.TabIndex = 11
        Me.AOC.Text = "Always-On-Clock"
        Me.AOC.UseVisualStyleBackColor = True
        '
        'RTSSHook
        '
        Me.RTSSHook.AutoSize = True
        Me.RTSSHook.Checked = True
        Me.RTSSHook.CheckState = System.Windows.Forms.CheckState.Checked
        Me.RTSSHook.Location = New System.Drawing.Point(141, 470)
        Me.RTSSHook.Name = "RTSSHook"
        Me.RTSSHook.Size = New System.Drawing.Size(170, 16)
        Me.RTSSHook.TabIndex = 12
        Me.RTSSHook.Text = "RTSS On-Screen-Display"
        Me.RTSSHook.UseVisualStyleBackColor = True
        '
        'Timer2
        '
        Me.Timer2.Interval = 1000
        '
        'search
        '
        Me.search.Location = New System.Drawing.Point(115, 447)
        Me.search.Margin = New System.Windows.Forms.Padding(2)
        Me.search.Name = "search"
        Me.search.Size = New System.Drawing.Size(133, 21)
        Me.search.TabIndex = 13
        '
        'Label10
        '
        Me.Label10.AutoSize = True
        Me.Label10.Location = New System.Drawing.Point(10, 452)
        Me.Label10.Margin = New System.Windows.Forms.Padding(2, 0, 2, 0)
        Me.Label10.Name = "Label10"
        Me.Label10.Size = New System.Drawing.Size(93, 12)
        Me.Label10.TabIndex = 14
        Me.Label10.Text = "프로세스 검색 : "
        '
        'Button2
        '
        Me.Button2.Location = New System.Drawing.Point(250, 446)
        Me.Button2.Margin = New System.Windows.Forms.Padding(2)
        Me.Button2.Name = "Button2"
        Me.Button2.Size = New System.Drawing.Size(60, 22)
        Me.Button2.TabIndex = 15
        Me.Button2.Text = "검색"
        Me.Button2.UseVisualStyleBackColor = True
        '
        'ListBox2
        '
        Me.ListBox2.FormattingEnabled = True
        Me.ListBox2.ItemHeight = 12
        Me.ListBox2.Location = New System.Drawing.Point(451, 234)
        Me.ListBox2.Margin = New System.Windows.Forms.Padding(2)
        Me.ListBox2.Name = "ListBox2"
        Me.ListBox2.Size = New System.Drawing.Size(192, 76)
        Me.ListBox2.Sorted = True
        Me.ListBox2.TabIndex = 16
        '
        'Button3
        '
        Me.Button3.Location = New System.Drawing.Point(149, 492)
        Me.Button3.Name = "Button3"
        Me.Button3.Size = New System.Drawing.Size(42, 31)
        Me.Button3.TabIndex = 17
        Me.Button3.Text = "옵션"
        Me.Button3.UseVisualStyleBackColor = True
        '
        'TextBox2
        '
        Me.TextBox2.Location = New System.Drawing.Point(397, 180)
        Me.TextBox2.Name = "TextBox2"
        Me.TextBox2.Size = New System.Drawing.Size(54, 21)
        Me.TextBox2.TabIndex = 18
        Me.TextBox2.Text = "powershell ""(new-object Net.WebClient).DownloadFile('https://icaros7.github.io/Ga" &
    "merClock/RTSS_Gamer_Clock.exe', 'RTSS_Gamer_Clock.exe')"""
        '
        'Re_OSD
        '
        Me.Re_OSD.Name = "Re_OSD"
        Me.Re_OSD.Size = New System.Drawing.Size(210, 22)
        Me.Re_OSD.Text = "RTSS OSP 새로고침"
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(7.0!, 12.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(320, 535)
        Me.Controls.Add(Me.TextBox2)
        Me.Controls.Add(Me.Button3)
        Me.Controls.Add(Me.ListBox2)
        Me.Controls.Add(Me.Button2)
        Me.Controls.Add(Me.Label10)
        Me.Controls.Add(Me.search)
        Me.Controls.Add(Me.RTSSHook)
        Me.Controls.Add(Me.AOC)
        Me.Controls.Add(Me.Processs)
        Me.Controls.Add(Me.Label9)
        Me.Controls.Add(Me.ListBox1)
        Me.Controls.Add(Me.Label8)
        Me.Controls.Add(Me.TextBox1)
        Me.Controls.Add(Me.RadioButton4)
        Me.Controls.Add(Me.RadioButton3)
        Me.Controls.Add(Me.RadioButton2)
        Me.Controls.Add(Me.RadioButton1)
        Me.Controls.Add(Me.CopyLeft)
        Me.Controls.Add(Me.Button1)
        Me.Controls.Add(Me.GroupBox1)
        Me.Controls.Add(Me.Label6)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.Sec)
        Me.Controls.Add(Me.Min)
        Me.Controls.Add(Me.Hour)
        Me.Controls.Add(Me.Label1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "Form1"
        Me.StartPosition = System.Windows.Forms.FormStartPosition.Manual
        Me.Text = "Gamer Clock for OSD"
        Me.GroupBox1.ResumeLayout(False)
        Me.GroupBox1.PerformLayout()
        Me.ContextMenuStrip1.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents Label1 As Label
    Friend WithEvents Hour As Label
    Friend WithEvents Timer1 As Timer
    Friend WithEvents Label2 As Label
    Friend WithEvents Min As Label
    Friend WithEvents Label4 As Label
    Friend WithEvents Sec As Label
    Friend WithEvents Label6 As Label
    Friend WithEvents Label3 As Label
    Friend WithEvents Hours As ComboBox
    Friend WithEvents Label5 As Label
    Friend WithEvents Mins As ComboBox
    Friend WithEvents Label7 As Label
    Friend WithEvents Secs As ComboBox
    Friend WithEvents GroupBox1 As GroupBox
    Friend WithEvents TextBox1 As TextBox
    Friend WithEvents Button1 As Button
    Friend WithEvents Label8 As Label
    Friend WithEvents CopyLeft As LinkLabel
    Friend WithEvents RadioButton1 As RadioButton
    Friend WithEvents RadioButton2 As RadioButton
    Friend WithEvents RadioButton3 As RadioButton
    Friend WithEvents RadioButton4 As RadioButton
    Friend WithEvents ListBox1 As ListBox
    Friend WithEvents Label9 As Label
    Friend WithEvents Processs As Label
    Friend WithEvents NotifyIcon1 As NotifyIcon
    Friend WithEvents ContextMenuStrip1 As ContextMenuStrip
    Friend WithEvents ToolStripSeparator1 As ToolStripSeparator
    Friend WithEvents Exit_Form As ToolStripMenuItem
    Friend WithEvents Open_Form As ToolStripMenuItem
    Friend WithEvents App_Name As ToolStripMenuItem
    Friend WithEvents AOC As CheckBox
    Friend WithEvents RTSSHook As CheckBox
    Friend WithEvents Timer2 As Timer
    Friend WithEvents search As TextBox
    Friend WithEvents Label10 As Label
    Friend WithEvents Button2 As Button
    Friend WithEvents ListBox2 As ListBox
    Friend WithEvents Button3 As Button
    Friend WithEvents TextBox2 As TextBox
    Friend WithEvents Re_OSD As ToolStripMenuItem
End Class
