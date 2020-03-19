function Hoosier2057513R25B = importTIR(filename, dataLines)

%IMPORTFILE Import data from a text file
%  HOOSIER2057513R25B = IMPORTFILE(FILENAME) reads data from text file
%  FILENAME for the default selection.  Returns the data as a table.
%
%  HOOSIER2057513R25B = IMPORTFILE(FILE, DATALINES) reads data for the
%  specified row interval(s) of text file FILENAME. Specify DATALINES as
%  a positive scalar integer or a N-by-2 array of positive scalar
%  integers for dis-contiguous row intervals.
%
%  Example:
%  Hoosier2057513R25B = importfile("C:\Users\Bob\Documents\EPSA\Ressources2020\SU_Suspension\02_singleTrack_2wheels_dynamicModel_simulink\tyres\Hoosier_20,5-7,5-13_R25B.TIR", [47, 47]);
%
%  See also READTABLE.
%
% Auto-generated by MATLAB on 27-Feb-2020 15:01:59

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [47, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 4);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = ["$", "="];

% Specify column names and types
opts.VariableNames = ["Name", "Value", "Var3", "Var4"];
opts.SelectedVariableNames = ["Name", "Value"];
opts.VariableTypes = ["string", "string", "string", "string"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, ["Name", "Value", "Var3", "Var4"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Name", "Value", "Var3", "Var4"], "EmptyFieldRule", "auto");

% Import the data
Hoosier2057513R25B = readtable(filename, opts);

end