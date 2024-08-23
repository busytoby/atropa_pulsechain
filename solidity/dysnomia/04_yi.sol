// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./include/bao.sol";
import "./interfaces/02d_shafactoryinterface.sol";
import "./interfaces/03b_shiointerface.sol";
import "./interfaces/03d_shiofactoryinterface.sol";

contract YI is DYSNOMIA {
    string public constant Type = "YI";
        
    SHIO public Psi;
    mapping(address => Bao) private Nu;
    uint64 public Xi;
    uint64 public Ring;
    SHAFactory public SHAFactoryInterface;
    SHIOFactory public SHIOFactoryInterface;

    constructor(address _shaFactory, address _shioFactory, address MathLib) DYSNOMIA(unicode"CHATLOG Yi", unicode"YI", MathLib) {
        SHAFactoryInterface = SHAFactory(_shaFactory);
        SHIOFactoryInterface = SHIOFactory(_shioFactory);
        SHA Rod = SHAFactoryInterface.New("Shio Rod", "SROD", MathLib);
        SHA Cone = SHAFactoryInterface.New("Shio Cone", "SCONE", MathLib);
        Psi = SHIOFactoryInterface.New(address(Rod), address(Cone), MathLib);
        Xi = Xiao.Random();
        Psi.Generate(Xi, Xiao.Random(), Xiao.Random());
        Ionize();
        Ring = Psi.Magnetize();
        Augment();
    }

    function Augment() internal {
        AddMarketRate(address(Psi), 1 * 10 ** decimals());
        AddMarketRate(address(Psi.Rod()), 1 * 10 ** decimals());
        AddMarketRate(address(Psi.Cone()), 1 * 10 ** decimals());
        _mintToCap();
    }

    function Beta(string calldata Name, string calldata Symbol) public onlyOwners returns(SHA) {
        SHA Xun = SHAFactoryInterface.New(Name, Symbol, address(Xiao));
        Xun.addOwner(msg.sender);
        _mintToCap();
        return Xun;
    }

    function Kappa(SHA Rod, SHA Cone) public onlyOwners returns(SHIO) {
        SHIO Xie = SHIOFactoryInterface.New(address(Rod), address(Cone), address(Xiao));
        Xie.addOwner(msg.sender);
        _mintToCap();
        return Xie;
    }

    function Bing(Bao memory _b) public onlyOwners {
        Nu[_b.Phi] = _b;
        SHIO(_b.Shio).Log(0, 0, "Added to Yi.Nu");
        _mintToCap();
    }

    function Bang(address _a) public view returns(Bao memory) {
        return Nu[_a];
    }
    
    function Ionize() private {
        Psi.Isomerize();
        Psi.Isolate();
        _mintToCap();
    }

    function React(Bao memory Gamma, uint64 Pi) public returns(Bao memory) {
        (Gamma.Omicron, Gamma.Omega) = SHIO(Gamma.Shio).React(Pi);
        _mintToCap();
        return Gamma;
    }
}