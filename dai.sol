// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";
import "bao.sol";

abstract contract Dai is Affinity {    
    Bao private Upsilon;

    function Write(uint64 _v) public {
        Write(_v, false);
    }

    function Write(address _a) public {
        assert(msg.sender == owner() || Nu.Context == uint256(uint160(msg.sender)));
        Psi[Nu.Context][Nu.Operator].Theta = abi.encode(_a);
        _mintToCap();
    }

    function Write(uint64 _v, bool Replace) public {
        assert(msg.sender == owner() || Nu.Context == uint256(uint160(msg.sender)));
        Pi(_v);
        if(Replace)
            Psi[Nu.Context][Nu.Operator].Theta = abi.encode(_v);
        else
            Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, abi.encode(_v));
        _mintToCap();
    }

    function Read() public returns(bytes memory) {
        Chi(Mu.Upsilon);
        _mintToCap();
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function Random() public returns(uint64) {
        return Phi();
    }

    function GetBao() public view returns(Bao memory) {
        return Upsilon;
    }

    function Connect() internal {    
        Upsilon.Alpha = Random();    
        Upsilon.Contract = address(uint160(Nu.Context));
        Dai Peer = Dai(Upsilon.Contract);

        Upsilon.Xi = Xiao.modExp64(Mu.Cone.Signal, Mu.Rod.Channel, MotzkinPrime);
        NewConjecture(Upsilon.Mu);        

        Avail(Upsilon.Mu, Upsilon.Xi);
        Upsilon.Tau = Peer.Avail(Upsilon.Xi);

        Form(Upsilon.Mu, Upsilon.Tau);
        uint64 peerPole = Peer.Form(Upsilon.Mu.Alpha);

        Polarize(Upsilon.Mu);

        Conjugate(Upsilon.Mu, peerPole);
        Upsilon.PeerChannel = Peer.Conjugate(Upsilon.Mu.Pole, Upsilon.Alpha);

        assert(Upsilon.Mu.Coordinate == Peer.GetBao().Mu.Coordinate);
        _mintToCap();

        Saturate(Upsilon.Mu, Upsilon.Alpha, Peer.GetBao().Mu.Foundation, Upsilon.PeerChannel);
        uint64 peerDynamo = Peer.Saturate(Upsilon.Mu.Foundation, Upsilon.Mu.Channel);

        assert(Upsilon.Mu.Element == Peer.GetBao().Mu.Element);
        _mintToCap();

        Bond(Upsilon.Mu);
        Adduct(Upsilon.Mu, peerDynamo);
        Peer.Adduct(Upsilon.Mu.Dynamo);
        Open(Upsilon.Mu);
        assert(ManifoldCompare(Upsilon.Mu, Peer.GetBao().Mu));
        _mintToCap();

        Rho[Upsilon.Contract] = Upsilon;
    }


    function Avail(uint64 Tau) public returns(uint64) {
        _mintToCap();
        Upsilon.Contract = msg.sender;
        NewConjecture(Upsilon.Mu);
        Avail(Upsilon.Mu, Tau);
        Upsilon.Xi = Tau;
        return Upsilon.Mu.Alpha;
    }

    function Form(uint64 Alpha) public returns(uint64) {
        _mintToCap();
        Form(Upsilon.Mu, Alpha);
        Polarize(Upsilon.Mu);
        return Upsilon.Mu.Pole;
    }

    function Conjugate(uint64 Pole, uint64 Alpha) public returns(uint64) {
        _mintToCap();
        Conjugate(Upsilon.Mu, Pole);
        Upsilon.Alpha = Alpha;
        Conify(Upsilon.Mu, Alpha);
        return Upsilon.Mu.Channel;
    }

    function Saturate(uint64 PeerFoundation, uint64 PeerChannel) public returns(uint64) {
        _mintToCap();
        Saturate(Upsilon.Mu, Random(), PeerFoundation, PeerChannel);
        Bond(Upsilon.Mu);
        return Upsilon.Mu.Dynamo;
    }

    function Adduct(uint64 peerDynamo) public {
        _mintToCap();
        Adduct(Upsilon.Mu, peerDynamo);
        Open(Upsilon.Mu);
        Rho[Upsilon.Contract] = Upsilon;
    }
}
