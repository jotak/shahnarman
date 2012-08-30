cost = {2,0,0,3}
icon = "elephant"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Invoquer troll aquatique"
	else
		return "Summon water-troll"
	end
end

function getDescription()
	if language == "french" then
		return "Invoque un horrible troll aquatique.\n"..getUnitDescription("Troll_aquatique_farceur")
	else
		return "Summons a horrible water-troll.\n"..getUnitDescription("Troll_aquatique_farceur")
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Troll_aquatique_farceur")
end
