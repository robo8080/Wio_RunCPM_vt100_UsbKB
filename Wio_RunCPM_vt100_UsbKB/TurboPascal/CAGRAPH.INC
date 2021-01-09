{* CAGRAPH.INC *}
{ https://oku.edu.mie-u.ac.jp/~okumura/algo/algo_pas.html }

var
  LastX, LastY: Real;

procedure CA_Dot(X, Y: Integer);
begin
  DrawPixel(x, y);
end;

procedure CA_Line(X1, Y1, X2, Y2: Integer);
begin
  DrawLine(X1, Y1, X2, Y2);
end;

procedure CA_Circle(Xcenter, Ycenter, Radius: Integer);
begin
  DrawCircle(Xcenter, Ycenter, Radius);
end;

procedure CA_MoveAbsolute(X, Y: Real);
begin
  LastX := X;
  LastY := Y;
end;

procedure CA_InitPlot;
begin
  CA_MoveAbsolute(0, 0);
end;

procedure CA_DrawAbsolute(X, Y: Real);
begin
  CA_Line(Round(LastX), Round(LastY), Round(X), Round(Y));
  CA_MoveAbsolute(X, Y);
end;

procedure CA_MoveRelative(X, Y: Real);
begin
  CA_MoveAbsolute(LastX + X, LastY + Y);
end;

procedure CA_DrawRelative(X, Y: Real);
begin
  CA_DrawAbsolute(LastX + X, LastY + Y);
end;

procedure CA_DrawCircle(R: Real);
begin
  CA_Circle(Round(LastX), Round(LastY), Round(R));
end;
