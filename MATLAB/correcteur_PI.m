clc;
clear all;

E=0;
R=1;
R21=220;
R5=5100;
R8=10000;
C2=22e-9;
L=2e-3;
K1=1/R;
K2=1+12/10;
K3=R8/(R5+R8);
H=48*0.104;
K_G=H*K1*K2*K3;

t1=L/R;
t2=R21*C2;
t3=(R5*R8*C2)/(R5+R8);

% Moteur
G1=tf(K1,[t1 1]);

% Filtre HF
G2=tf(K2,[t2 1]);

%Filtre BF
G3=tf(K3,[t3 1]);

% tout le système
G =H*G1*G2*G3;
bode(G)
P=pole(G);

% Correcteur continu
Kci = 2*pi*400/(K_G)
Kcp=Kci/(80*2*pi);
C = pid(Kcp,Kci,0,0);
bode(C*G);
syst = feedback(C*G,1);
%bode(syst)

%Correcteur discret
Te=1/800; %A régler tq Shannon
T1=Kcp/Kci;
T2=1/Kci;
Cz=c2d(C,Te,'tustin');
