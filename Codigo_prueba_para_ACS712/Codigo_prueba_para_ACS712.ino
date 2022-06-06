#define A              5    // Variable de menor peso del mux
#define B              6    // Variable de medio peso del mux
#define C              7    // Variable de mayor peso del mux

void setup() {
Serial.begin(9600);
}
void loop() {
float voltajeSensor =get_voltage(3000);//obtenemos voltaje del sensor(10000 muestras)
Serial.print("Voltaje del sensor: ");
Serial.println(voltajeSensor ,3);

    digitalWrite(A, HIGH); //0   0
    digitalWrite(B, HIGH); //0
    digitalWrite(C, LOW); //0

}
float get_voltage(int n_muestras)
{
float voltage=0;
for(int i=0;i<n_muestras;i++)
{
voltage =voltage+analogRead(A0) * (5.21 / 1023.0);
}
voltage=voltage/n_muestras;
return(voltage);
}
